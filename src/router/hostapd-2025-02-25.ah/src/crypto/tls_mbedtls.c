/*
 * SSL/TLS interface functions for mbed TLS
 *
 * SPDX-FileCopyrightText: 2022 Glenn Strauss <gstrauss@gluelogic.com>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * template:  src/crypto/tls_none.c
 * reference: src/crypto/tls_*.c
 *
 * Known Limitations:
 * - no TLSv1.3 (not available in mbedtls 2.x; experimental in mbedtls 3.x)
 * - no OCSP (not yet available in mbedtls)
 * - mbedtls does not support all certificate encodings used by hwsim tests
 *   PCKS#5 v1.5
 *   PCKS#12
 *   DH DSA
 * - EAP-FAST, EAP-TEAP session ticket support not implemented in tls_mbedtls.c
 * - mbedtls does not currently provide way to set an attribute in a CSR
 *     https://github.com/Mbed-TLS/mbedtls/issues/4886
 *   so tests/hwsim dpp_enterprise tests fail
 * - DPP2 not supported
 *   PKCS#7 parsing is not supported in mbedtls
 *   See crypto_mbedtls.c:crypto_pkcs7_get_certificates() comments
 * - DPP3 not supported
 *   hpke_base_seal() and hpke_base_seal() not implemented in crypto_mbedtls.c
 *
 * Status:
 * - code written to be compatible with mbedtls 2.x and mbedtls 3.x
 *   (currently requires mbedtls >= 2.27.0 for mbedtls_mpi_random())
 *   (currently requires mbedtls >= 2.18.0 for mbedtls_ssl_tls_prf())
 * - builds with tests/build/build-wpa_supplicant-mbedtls.config
 * - passes all tests/ crypto module tests (incomplete coverage)
 *   ($ cd tests; make clean; make -j 4 run-tests CONFIG_TLS=mbedtls)
 * - passes almost all tests/hwsim tests
 *   (hwsim tests skipped for missing features)
 *
 * RFE:
 * - EAP-FAST, EAP-TEAP session ticket support not implemented in tls_mbedtls.c
 * - client/server session resumption, and/or save client session ticket
 */

#include "includes.h"
#include "common.h"

#include <mbedtls/version.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/oid.h>
#include <mbedtls/pem.h>
#include <mbedtls/platform.h> /* mbedtls_calloc() mbedtls_free() */
#include <mbedtls/platform_util.h> /* mbedtls_platform_zeroize() */
#include <mbedtls/ssl.h>
#include <mbedtls/ssl_ticket.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_crt.h>

#if MBEDTLS_VERSION_NUMBER >= 0x02040000 /* mbedtls 2.4.0 */
#include <mbedtls/net_sockets.h>
#else
#include <mbedtls/net.h>
#endif

#ifndef MBEDTLS_PRIVATE
#define MBEDTLS_PRIVATE(x) x
#endif

#if MBEDTLS_VERSION_NUMBER < 0x03020000 /* mbedtls 3.2.0 */
#define mbedtls_ssl_get_ciphersuite_id_from_ssl(ssl) \
        ((ssl)->MBEDTLS_PRIVATE(session) \
        ?(ssl)->MBEDTLS_PRIVATE(session)->MBEDTLS_PRIVATE(ciphersuite) \
        : 0)
#define mbedtls_ssl_ciphersuite_get_name(info) \
        (info)->MBEDTLS_PRIVATE(name)
#endif

#include "crypto.h"     /* sha256_vector() */
#include "tls.h"

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif

#ifndef MBEDTLS_EXPKEY_FIXED_SECRET_LEN
#define MBEDTLS_EXPKEY_FIXED_SECRET_LEN 48
#endif

#ifndef MBEDTLS_EXPKEY_RAND_LEN
#define MBEDTLS_EXPKEY_RAND_LEN 32
#endif

#if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
static mbedtls_ssl_export_keys_t tls_connection_export_keys_cb;
#elif MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */
static mbedtls_ssl_export_keys_ext_t tls_connection_export_keys_cb;
#else /*(not implemented; return error)*/
#define mbedtls_ssl_tls_prf(a,b,c,d,e,f,g,h) (-1)
typedef mbedtls_tls_prf_types int;
#endif


/* hostapd/wpa_supplicant provides forced_memzero(),
 * but prefer mbedtls_platform_zeroize() */
#define forced_memzero(ptr,sz) mbedtls_platform_zeroize(ptr,sz)


#if defined(EAP_FAST) || defined(EAP_FAST_DYNAMIC) || defined(EAP_SERVER_FAST) \
 || defined(EAP_TEAP) || defined(EAP_SERVER_TEAP)
#ifdef MBEDTLS_SSL_SESSION_TICKETS
#ifdef MBEDTLS_SSL_TICKET_C
#define TLS_MBEDTLS_SESSION_TICKETS
#if defined(EAP_TEAP) || defined(EAP_SERVER_TEAP)
#define TLS_MBEDTLS_EAP_TEAP
#endif
#if !defined(CONFIG_FIPS) /* EAP-FAST keys cannot be exported in FIPS mode */
#if defined(EAP_FAST) || defined(EAP_FAST_DYNAMIC) || defined(EAP_SERVER_FAST)
#define TLS_MBEDTLS_EAP_FAST
#endif
#endif
#endif
#endif
#endif


struct tls_conf {
	mbedtls_ssl_config conf;

	unsigned int verify_peer:1;
	unsigned int verify_depth0_only:1;
	unsigned int check_crl:2;           /*(needs :2 bits for 0, 1, 2)*/
	unsigned int check_crl_strict:1;    /*(needs :1 bit  for 0, 1)*/
	unsigned int ca_cert_probe:1;
	unsigned int has_ca_cert:1;
	unsigned int has_client_cert:1;
	unsigned int has_private_key:1;
	unsigned int suiteb128:1;
	unsigned int suiteb192:1;
	mbedtls_x509_crl *crl;
	mbedtls_x509_crt ca_cert;
	mbedtls_x509_crt client_cert;
	mbedtls_pk_context private_key;

	uint32_t refcnt;

	unsigned int flags;
	char *subject_match;
	char *altsubject_match;
	char *suffix_match;
	char *domain_match;
	char *check_cert_subject;
	u8 ca_cert_hash[SHA256_DIGEST_LENGTH];

	int *ciphersuites;  /* list of ciphersuite ids for mbedtls_ssl_config */
#if MBEDTLS_VERSION_NUMBER < 0x03010000 /* mbedtls 3.1.0 */
	mbedtls_ecp_group_id *curves;
#else
	uint16_t *curves;   /* list of curve ids for mbedtls_ssl_config */
#endif
};


struct tls_global {
	struct tls_conf *tls_conf;
	char *ocsp_stapling_response;
	mbedtls_ctr_drbg_context *ctr_drbg; /*(see crypto_mbedtls.c)*/
  #ifdef MBEDTLS_SSL_SESSION_TICKETS
	mbedtls_ssl_ticket_context ticket_ctx;
  #endif
	char *ca_cert_file;
	struct os_reltime crl_reload_previous;
	unsigned int crl_reload_interval;
	uint32_t refcnt;
	struct tls_config init_conf;
};

static struct tls_global tls_ctx_global;


struct tls_connection {
	struct tls_conf *tls_conf;
	struct wpabuf *push_buf;
	struct wpabuf *pull_buf;
	size_t pull_buf_offset;

	unsigned int established:1;
	unsigned int resumed:1;
	unsigned int verify_peer:1;
	unsigned int is_server:1;

	mbedtls_ssl_context ssl;

	mbedtls_tls_prf_types tls_prf_type;
	size_t expkey_keyblock_size;
	size_t expkey_secret_len;
  #if MBEDTLS_VERSION_NUMBER < 0x03000000 /* mbedtls 3.0.0 */
	unsigned char expkey_secret[MBEDTLS_EXPKEY_FIXED_SECRET_LEN];
  #else
	unsigned char expkey_secret[MBEDTLS_MD_MAX_SIZE];
  #endif
	unsigned char expkey_randbytes[MBEDTLS_EXPKEY_RAND_LEN*2];

	int read_alerts, write_alerts, failed;

  #ifdef TLS_MBEDTLS_SESSION_TICKETS
	tls_session_ticket_cb session_ticket_cb;
	void *session_ticket_cb_ctx;
	unsigned char *clienthello_session_ticket;
	size_t clienthello_session_ticket_len;
  #endif
	char *peer_subject; /* peer subject info for authenticated peer */
	struct wpabuf *success_data;
};


#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(maj,min) 0
#endif

#ifndef __attribute_cold__
#if __has_attribute(cold) \
 || __GNUC_PREREQ(4,3)
#define __attribute_cold__  __attribute__((__cold__))
#else
#define __attribute_cold__
#endif
#endif

#ifndef __attribute_noinline__
#if __has_attribute(noinline) \
 || __GNUC_PREREQ(3,1)
#define __attribute_noinline__  __attribute__((__noinline__))
#else
#define __attribute_noinline__
#endif
#endif


__attribute_cold__
__attribute_noinline__
static void emsg(int level, const char * const msg)
{
	wpa_printf(level, "MTLS: %s", msg);
}


__attribute_cold__
__attribute_noinline__
static void emsgrc(int level, const char * const msg, int rc)
{
  #ifdef MBEDTLS_ERROR_C
	/* error logging convenience function that decodes mbedtls result codes */
	char buf[256];
	mbedtls_strerror(rc, buf, sizeof(buf));
	wpa_printf(level, "MTLS: %s: %s (-0x%04x)", msg, buf, -rc);
  #else
	wpa_printf(level, "MTLS: %s: (-0x%04x)", msg, -rc);
  #endif
}


#define elog(rc, msg) emsgrc(MSG_ERROR, (msg), (rc))
#define ilog(rc, msg) emsgrc(MSG_INFO,  (msg), (rc))


struct tls_conf * tls_conf_init(void *tls_ctx)
{
	struct tls_conf *tls_conf = os_zalloc(sizeof(*tls_conf));
	if (tls_conf == NULL)
		return NULL;
	tls_conf->refcnt = 1;

	mbedtls_ssl_config_init(&tls_conf->conf);
	mbedtls_ssl_conf_rng(&tls_conf->conf,
			     mbedtls_ctr_drbg_random, tls_ctx_global.ctr_drbg);
	mbedtls_x509_crt_init(&tls_conf->ca_cert);
	mbedtls_x509_crt_init(&tls_conf->client_cert);
	mbedtls_pk_init(&tls_conf->private_key);

	return tls_conf;
}


void tls_conf_deinit(struct tls_conf *tls_conf)
{
	if (tls_conf == NULL || --tls_conf->refcnt != 0)
		return;

	mbedtls_x509_crt_free(&tls_conf->ca_cert);
	mbedtls_x509_crt_free(&tls_conf->client_cert);
	if (tls_conf->crl) {
		mbedtls_x509_crl_free(tls_conf->crl);
		os_free(tls_conf->crl);
	}
	mbedtls_pk_free(&tls_conf->private_key);
	mbedtls_ssl_config_free(&tls_conf->conf);
	os_free(tls_conf->curves);
	os_free(tls_conf->ciphersuites);
	os_free(tls_conf->subject_match);
	os_free(tls_conf->altsubject_match);
	os_free(tls_conf->suffix_match);
	os_free(tls_conf->domain_match);
	os_free(tls_conf->check_cert_subject);
	os_free(tls_conf);
}


mbedtls_ctr_drbg_context * crypto_mbedtls_ctr_drbg(void); /*(not in header)*/

__attribute_cold__
void * tls_init(const struct tls_config *conf)
{
	/* RFE: review struct tls_config *conf (different from tls_conf) */

	if (++tls_ctx_global.refcnt > 1)
		return &tls_ctx_global;

	tls_ctx_global.ctr_drbg = crypto_mbedtls_ctr_drbg();
  #ifdef MBEDTLS_SSL_SESSION_TICKETS
	mbedtls_ssl_ticket_init(&tls_ctx_global.ticket_ctx);
	mbedtls_ssl_ticket_setup(&tls_ctx_global.ticket_ctx,
	                         mbedtls_ctr_drbg_random,
	                         tls_ctx_global.ctr_drbg,
	                         MBEDTLS_CIPHER_AES_256_GCM,
	                         43200); /* ticket timeout: 12 hours */
  #endif
	/* copy struct for future use */
	tls_ctx_global.init_conf = *conf;
	if (conf->openssl_ciphers)
		tls_ctx_global.init_conf.openssl_ciphers =
		  os_strdup(conf->openssl_ciphers);

	tls_ctx_global.crl_reload_interval = conf->crl_reload_interval;
	os_get_reltime(&tls_ctx_global.crl_reload_previous);

	return &tls_ctx_global;
}


__attribute_cold__
void tls_deinit(void *tls_ctx)
{
	if (tls_ctx == NULL || --tls_ctx_global.refcnt != 0)
		return;

	tls_conf_deinit(tls_ctx_global.tls_conf);
	os_free(tls_ctx_global.ca_cert_file);
	os_free(tls_ctx_global.ocsp_stapling_response);
	char *openssl_ciphers; /*(allocated in tls_init())*/
	*(const char **)&openssl_ciphers =
	  tls_ctx_global.init_conf.openssl_ciphers;
	os_free(openssl_ciphers);
  #ifdef MBEDTLS_SSL_SESSION_TICKETS
	mbedtls_ssl_ticket_free(&tls_ctx_global.ticket_ctx);
  #endif
	os_memset(&tls_ctx_global, 0, sizeof(tls_ctx_global));
}


int tls_get_errors(void *tls_ctx)
{
	return 0;
}


static void tls_connection_deinit_expkey(struct tls_connection *conn)
{
	conn->tls_prf_type = 0; /* MBEDTLS_SSL_TLS_PRF_NONE; */
	conn->expkey_keyblock_size = 0;
	conn->expkey_secret_len = 0;
	forced_memzero(conn->expkey_secret, sizeof(conn->expkey_secret));
	forced_memzero(conn->expkey_randbytes, sizeof(conn->expkey_randbytes));
}


#ifdef TLS_MBEDTLS_SESSION_TICKETS
void tls_connection_deinit_clienthello_session_ticket(struct tls_connection *conn)
{
	if (conn->clienthello_session_ticket) {
		mbedtls_platform_zeroize(conn->clienthello_session_ticket,
		                         conn->clienthello_session_ticket_len);
		mbedtls_free(conn->clienthello_session_ticket);
		conn->clienthello_session_ticket = NULL;
		conn->clienthello_session_ticket_len = 0;
	}
}
#endif


void tls_connection_deinit(void *tls_ctx, struct tls_connection *conn)
{
	if (conn == NULL)
		return;

  #if 0 /*(good intention, but never sent since we destroy self below)*/
	if (conn->established)
		mbedtls_ssl_close_notify(&conn->ssl);
  #endif

	if (conn->tls_prf_type)
		tls_connection_deinit_expkey(conn);

  #ifdef TLS_MBEDTLS_SESSION_TICKETS
	if (conn->clienthello_session_ticket)
		tls_connection_deinit_clienthello_session_ticket(conn);
  #endif

	os_free(conn->peer_subject);
	wpabuf_free(conn->success_data);
	wpabuf_free(conn->push_buf);
	wpabuf_free(conn->pull_buf);
	mbedtls_ssl_free(&conn->ssl);
	tls_conf_deinit(conn->tls_conf);
	os_free(conn);
}


static void tls_mbedtls_refresh_crl(void);
static int tls_mbedtls_ssl_setup(struct tls_connection *conn);

struct tls_connection * tls_connection_init(void *tls_ctx)
{
	struct tls_connection *conn = os_zalloc(sizeof(*conn));
	if (conn == NULL)
		return NULL;

	mbedtls_ssl_init(&conn->ssl);

	conn->tls_conf = tls_ctx_global.tls_conf; /*(inherit global conf, if set)*/
	if (conn->tls_conf) {
		++conn->tls_conf->refcnt;
		/* check for CRL refresh if inheriting from global config */
		tls_mbedtls_refresh_crl();

		conn->verify_peer = conn->tls_conf->verify_peer;
		if (tls_mbedtls_ssl_setup(conn) != 0) {
			tls_connection_deinit(&tls_ctx_global, conn);
			return NULL;
		}
	}

	return conn;
}


int tls_connection_established(void *tls_ctx, struct tls_connection *conn)
{
	return conn ? conn->established : 0;
}


__attribute_noinline__
char * tls_mbedtls_peer_serial_num(const mbedtls_x509_crt *crt, char *serial_num, size_t len)
{
	/* mbedtls_x509_serial_gets() inefficiently formats to hex separated by
	 * colons, so generate the hex serial number here.  The func
	 * wpa_snprintf_hex_uppercase() is similarly inefficient. */
	size_t i = 0; /* skip leading 0's per Distinguished Encoding Rules (DER) */
	while (i < crt->serial.len && crt->serial.p[i] == 0) ++i;
	if (i == crt->serial.len) --i;

	const unsigned char *s = crt->serial.p + i;
	const size_t e = (crt->serial.len - i) * 2;
	if (e >= len)
		return NULL;
  #if 0
	wpa_snprintf_hex_uppercase(serial_num, len, s, crt->serial.len-i);
  #else
	for (i = 0; i < e; i+=2, ++s) {
		serial_num[i+0] = "0123456789ABCDEF"[(*s >>  4)];
		serial_num[i+1] = "0123456789ABCDEF"[(*s & 0xF)];
	}
	serial_num[e] = '\0';
  #endif
	return serial_num;
}


char * tls_connection_peer_serial_num(void *tls_ctx,
				      struct tls_connection *conn)
{
	const mbedtls_x509_crt *crt = mbedtls_ssl_get_peer_cert(&conn->ssl);
	if (crt == NULL)
		return NULL;
	size_t len = crt->serial.len * 2 + 1;
	char *serial_num = os_malloc(len);
	if (!serial_num)
		return NULL;
	return tls_mbedtls_peer_serial_num(crt, serial_num, len);
}


static void tls_pull_buf_reset(struct tls_connection *conn);

int tls_connection_shutdown(void *tls_ctx, struct tls_connection *conn)
{
	/* Note: this function called from eap_peer_tls_reauth_init()
	 * for session resumption, not for connection shutdown */

	if (conn == NULL)
		return -1;

	tls_pull_buf_reset(conn);
	wpabuf_free(conn->push_buf);
	conn->push_buf = NULL;
	conn->established = 0;
	conn->resumed = 0;
	if (conn->tls_prf_type)
		tls_connection_deinit_expkey(conn);

	/* RFE: prepare for session resumption? (see doc in crypto/tls.h) */

	return mbedtls_ssl_session_reset(&conn->ssl);
}


static int tls_wpabuf_resize_put_data(struct wpabuf **buf,
                                      const unsigned char *data, size_t dlen)
{
	if (wpabuf_resize(buf, dlen) < 0)
		return 0;
	wpabuf_put_data(*buf, data, dlen);
	return 1;
}


static int tls_pull_buf_append(struct tls_connection *conn,
                               const struct wpabuf *in_data)
{
	/*(interface does not lend itself to move semantics)*/
	return tls_wpabuf_resize_put_data(&conn->pull_buf,
	                                  wpabuf_head(in_data),
	                                  wpabuf_len(in_data));
}


static void tls_pull_buf_reset(struct tls_connection *conn)
{
	/*(future: might consider reusing conn->pull_buf)*/
	wpabuf_free(conn->pull_buf);
	conn->pull_buf = NULL;
	conn->pull_buf_offset = 0;
}


__attribute_cold__
static void tls_pull_buf_discard(struct tls_connection *conn, const char *func)
{
	size_t discard = wpabuf_len(conn->pull_buf) - conn->pull_buf_offset;
	if (discard)
		wpa_printf(MSG_DEBUG,
			   "%s - %zu bytes remaining in pull_buf; discarding",
			   func, discard);
	tls_pull_buf_reset(conn);
}


static int tls_pull_func(void *ptr, unsigned char *buf, size_t len)
{
	struct tls_connection *conn = (struct tls_connection *) ptr;
	if (conn->pull_buf == NULL)
		return MBEDTLS_ERR_SSL_WANT_READ;
	const size_t dlen = wpabuf_len(conn->pull_buf) - conn->pull_buf_offset;
	if (dlen == 0)
		return MBEDTLS_ERR_SSL_WANT_READ;

	if (len > dlen)
		len = dlen;
	os_memcpy(buf, wpabuf_head(conn->pull_buf)+conn->pull_buf_offset, len);

	if (len == dlen) {
		tls_pull_buf_reset(conn);
		/*wpa_printf(MSG_DEBUG, "%s - emptied pull_buf", __func__);*/
	}
	else {
		conn->pull_buf_offset += len;
		/*wpa_printf(MSG_DEBUG, "%s - %zu bytes remaining in pull_buf",
			   __func__, dlen - len);*/
	}
	return (int)len;
}


static int tls_push_func(void *ptr, const unsigned char *buf, size_t len)
{
	struct tls_connection *conn = (struct tls_connection *) ptr;
	return tls_wpabuf_resize_put_data(&conn->push_buf, buf, len)
	  ? (int)len
	  : MBEDTLS_ERR_SSL_ALLOC_FAILED;
}


static int
tls_mbedtls_verify_cb (void *arg, mbedtls_x509_crt *crt, int depth, uint32_t *flags);


static int tls_mbedtls_ssl_setup(struct tls_connection *conn)
{
  #if 0
	/* mbedtls_ssl_setup() must be called only once */
	/* If this func might be called multiple times (e.g. via set_params),
	 * then we should set a flag in conn that ssl was initialized */
	if (conn->ssl_is_init) {
		mbedtls_ssl_free(&conn->ssl);
		mbedtls_ssl_init(&conn->ssl);
	}
  #endif

	int ret = mbedtls_ssl_setup(&conn->ssl, &conn->tls_conf->conf);
	if (ret != 0) {
		elog(ret, "mbedtls_ssl_setup");
		return -1;
	}

	mbedtls_ssl_set_bio(&conn->ssl, conn, tls_push_func, tls_pull_func, NULL);
  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	mbedtls_ssl_set_export_keys_cb(
	    &conn->ssl, tls_connection_export_keys_cb, conn);
  #elif MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */
	mbedtls_ssl_conf_export_keys_ext_cb(
	    &conn->tls_conf->conf, tls_connection_export_keys_cb, conn);
  #endif
	if (conn->verify_peer)
		mbedtls_ssl_set_verify(&conn->ssl, tls_mbedtls_verify_cb, conn);

	return 0;
}


static int tls_mbedtls_data_is_pem(const u8 *data)
{
    return (NULL != os_strstr((char *)data, "-----"));
}


static void tls_mbedtls_set_allowed_tls_vers(struct tls_conf *tls_conf,
                                             mbedtls_ssl_config *conf)
{
  #if !defined(MBEDTLS_SSL_PROTO_TLS1_3)
	tls_conf->flags |= TLS_CONN_DISABLE_TLSv1_3;
  #endif

	/* unconditionally require TLSv1.2+ for TLS_CONN_SUITEB */
	if (tls_conf->flags & TLS_CONN_SUITEB) {
		tls_conf->flags |= TLS_CONN_DISABLE_TLSv1_0;
		tls_conf->flags |= TLS_CONN_DISABLE_TLSv1_1;
	}

	const unsigned int flags = tls_conf->flags;

	/* attempt to map flags to min and max TLS protocol version */

	int min = (flags & TLS_CONN_DISABLE_TLSv1_0)
		? (flags & TLS_CONN_DISABLE_TLSv1_1)
		? (flags & TLS_CONN_DISABLE_TLSv1_2)
		? (flags & TLS_CONN_DISABLE_TLSv1_3)
		? 4
		: 3
		: 2
		: 1
		: 0;

	int max = (flags & TLS_CONN_DISABLE_TLSv1_3)
		? (flags & TLS_CONN_DISABLE_TLSv1_2)
		? (flags & TLS_CONN_DISABLE_TLSv1_1)
		? (flags & TLS_CONN_DISABLE_TLSv1_0)
		? -1
		: 0
		: 1
		: 2
		: 3;

	if ((flags & TLS_CONN_ENABLE_TLSv1_2) && min > 2) min = 2;
	if ((flags & TLS_CONN_ENABLE_TLSv1_1) && min > 1) min = 1;
	if ((flags & TLS_CONN_ENABLE_TLSv1_0) && min > 0) min = 0;
	if (max < min) {
		emsg(MSG_ERROR, "invalid tls_disable_tlsv* params; ignoring");
		return;
	}
  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	/* mbed TLS 3.0.0 removes support for protocols < TLSv1.2 */
	if (min < 2 || max < 2) {
		emsg(MSG_ERROR, "invalid tls_disable_tlsv* params; ignoring");
		if (min < 2) min = 2;
		if (max < 2) max = 2;
	}
  #endif

  #if MBEDTLS_VERSION_NUMBER >= 0x03020000 /* mbedtls 3.2.0 */
	/* MBEDTLS_SSL_VERSION_TLS1_2 = 0x0303 *//*!< (D)TLS 1.2 */
	/* MBEDTLS_SSL_VERSION_TLS1_3 = 0x0304 *//*!< (D)TLS 1.3 */
	min = (min == 2) ? MBEDTLS_SSL_VERSION_TLS1_2 : MBEDTLS_SSL_VERSION_TLS1_3;
	max = (max == 2) ? MBEDTLS_SSL_VERSION_TLS1_2 : MBEDTLS_SSL_VERSION_TLS1_3;
	mbedtls_ssl_conf_min_tls_version(conf, min);
	mbedtls_ssl_conf_max_tls_version(conf, max);
  #else
   #ifndef MBEDTLS_SSL_MINOR_VERSION_4
	if (min == 3) min = 2;
	if (max == 3) max = 2;
   #endif
	/* MBEDTLS_SSL_MINOR_VERSION_0  0 *//*!< SSL v3.0 */
	/* MBEDTLS_SSL_MINOR_VERSION_1  1 *//*!< TLS v1.0 */
	/* MBEDTLS_SSL_MINOR_VERSION_2  2 *//*!< TLS v1.1 */
	/* MBEDTLS_SSL_MINOR_VERSION_3  3 *//*!< TLS v1.2 */
	/* MBEDTLS_SSL_MINOR_VERSION_4  4 *//*!< TLS v1.3 */
	mbedtls_ssl_conf_min_version(conf, MBEDTLS_SSL_MAJOR_VERSION_3, min+1);
	mbedtls_ssl_conf_max_version(conf, MBEDTLS_SSL_MAJOR_VERSION_3, max+1);
  #endif
}


__attribute_noinline__
static int tls_mbedtls_readfile(const char *path, u8 **buf, size_t *n);


static int
tls_mbedtls_set_dhparams(struct tls_conf *tls_conf, const char *dh_file)
{
    size_t len;
    u8 *data;
    if (tls_mbedtls_readfile(dh_file, &data, &len))
        return 0;

    /* parse only if DH parameters if in PEM format */
    if (tls_mbedtls_data_is_pem(data)
        && NULL == os_strstr((char *)data, "-----BEGIN DH PARAMETERS-----")) {
        if (os_strstr((char *)data, "-----BEGIN DSA PARAMETERS-----"))
            wpa_printf(MSG_WARNING, "DSA parameters not handled (%s)", dh_file);
        else
            wpa_printf(MSG_WARNING, "unexpected DH param content (%s)",dh_file);
        forced_memzero(data, len);
        os_free(data);
        return 0;
    }

    /* mbedtls_dhm_parse_dhm() expects "-----BEGIN DH PARAMETERS-----" if PEM */
    mbedtls_dhm_context dhm;
    mbedtls_dhm_init(&dhm);
    int rc = mbedtls_dhm_parse_dhm(&dhm, data, len);
    if (0 == rc)
        rc = mbedtls_ssl_conf_dh_param_ctx(&tls_conf->conf, &dhm);
    if (0 != rc)
        elog(rc, dh_file);
    mbedtls_dhm_free(&dhm);

    forced_memzero(data, len);
    os_free(data);
    return (0 == rc);
}


/* reference: lighttpd src/mod_mbedtls.c:mod_mbedtls_ssl_append_curve()
 * (same author: gstrauss@gluelogic.com; same license: BSD-3-Clause) */
#if MBEDTLS_VERSION_NUMBER < 0x03010000 /* mbedtls 3.1.0 */
static int
tls_mbedtls_append_curve (mbedtls_ecp_group_id *ids, int nids, int idsz, const mbedtls_ecp_group_id id)
{
    if (1 >= idsz - (nids + 1)) {
        emsg(MSG_ERROR, "error: too many curves during list expand");
        return -1;
    }
    ids[++nids] = id;
    return nids;
}


static int
tls_mbedtls_set_curves(struct tls_conf *tls_conf, const char *curvelist)
{
    mbedtls_ecp_group_id ids[512];
    int nids = -1;
    const int idsz = (int)(sizeof(ids)/sizeof(*ids)-1);
    const mbedtls_ecp_curve_info * const curve_info = mbedtls_ecp_curve_list();

    for (const char *e = curvelist-1; e; ) {
        const char * const n = e+1;
        e = os_strchr(n, ':');
        size_t len = e ? (size_t)(e - n) : os_strlen(n);
        mbedtls_ecp_group_id grp_id = MBEDTLS_ECP_DP_NONE;
        switch (len) {
          case 5:
            if (0 == os_memcmp("P-521", n, 5))
                grp_id = MBEDTLS_ECP_DP_SECP521R1;
            else if (0 == os_memcmp("P-384", n, 5))
                grp_id = MBEDTLS_ECP_DP_SECP384R1;
            else if (0 == os_memcmp("P-256", n, 5))
                grp_id = MBEDTLS_ECP_DP_SECP256R1;
            break;
          case 6:
            if (0 == os_memcmp("BP-521", n, 6))
                grp_id = MBEDTLS_ECP_DP_BP512R1;
            else if (0 == os_memcmp("BP-384", n, 6))
                grp_id = MBEDTLS_ECP_DP_BP384R1;
            else if (0 == os_memcmp("BP-256", n, 6))
                grp_id = MBEDTLS_ECP_DP_BP256R1;
            break;
          default:
            break;
        }
        if (grp_id != MBEDTLS_ECP_DP_NONE) {
            nids = tls_mbedtls_append_curve(ids, nids, idsz, grp_id);
            if (-1 == nids) return 0;
            continue;
        }
        /* similar to mbedtls_ecp_curve_info_from_name() */
        const mbedtls_ecp_curve_info *info;
        for (info = curve_info; info->grp_id != MBEDTLS_ECP_DP_NONE; ++info) {
            if (0 == os_strncmp(info->name, n, len) && info->name[len] == '\0')
                break;
        }
        if (info->grp_id == MBEDTLS_ECP_DP_NONE) {
            wpa_printf(MSG_ERROR, "MTLS: unrecognized curve: %.*s",(int)len,n);
            return 0;
        }

        nids = tls_mbedtls_append_curve(ids, nids, idsz, info->grp_id);
        if (-1 == nids) return 0;
    }

    /* mod_openssl configures "prime256v1" if curve list not specified,
     * but mbedtls provides a list of supported curves if not explicitly set */
    if (-1 == nids) return 1; /* empty list; no-op */

    ids[++nids] = MBEDTLS_ECP_DP_NONE; /* terminate list */
    ++nids;

    /* curves list must be persistent for lifetime of mbedtls_ssl_config */
    tls_conf->curves = os_malloc(nids * sizeof(mbedtls_ecp_group_id));
    if (tls_conf->curves == NULL)
        return 0;
    os_memcpy(tls_conf->curves, ids, nids * sizeof(mbedtls_ecp_group_id));

    mbedtls_ssl_conf_curves(&tls_conf->conf, tls_conf->curves);
    return 1;
}
#else
static int
tls_mbedtls_append_curve (uint16_t *ids, int nids, int idsz, const uint16_t id)
{
    if (1 >= idsz - (nids + 1)) {
        emsg(MSG_ERROR, "error: too many curves during list expand");
        return -1;
    }
    ids[++nids] = id;
    return nids;
}


static int
tls_mbedtls_set_curves(struct tls_conf *tls_conf, const char *curvelist)
{
    /* TLS Supported Groups (renamed from "EC Named Curve Registry")
     * https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-8
     */
    uint16_t ids[512];
    int nids = -1;
    const int idsz = (int)(sizeof(ids)/sizeof(*ids)-1);
    const mbedtls_ecp_curve_info * const curve_info = mbedtls_ecp_curve_list();

    for (const char *e = curvelist-1; e; ) {
        const char * const n = e+1;
        e = os_strchr(n, ':');
        size_t len = e ? (size_t)(e - n) : os_strlen(n);
        uint16_t tls_id = 0;
        switch (len) {
          case 5:
            if (0 == os_memcmp("P-521", n, 5))
                tls_id = 25; /* mbedtls_ecp_group_id MBEDTLS_ECP_DP_SECP521R1 */
            else if (0 == os_memcmp("P-384", n, 5))
                tls_id = 24; /* mbedtls_ecp_group_id MBEDTLS_ECP_DP_SECP384R1 */
            else if (0 == os_memcmp("P-256", n, 5))
                tls_id = 23; /* mbedtls_ecp_group_id MBEDTLS_ECP_DP_SECP256R1 */
            break;
          case 6:
            if (0 == os_memcmp("BP-521", n, 6))
                tls_id = 28; /* mbedtls_ecp_group_id MBEDTLS_ECP_DP_BP512R1 */
            else if (0 == os_memcmp("BP-384", n, 6))
                tls_id = 27; /* mbedtls_ecp_group_id MBEDTLS_ECP_DP_BP384R1 */
            else if (0 == os_memcmp("BP-256", n, 6))
                tls_id = 26; /* mbedtls_ecp_group_id MBEDTLS_ECP_DP_BP256R1 */
            break;
          default:
            break;
        }
        if (tls_id != 0) {
            nids = tls_mbedtls_append_curve(ids, nids, idsz, tls_id);
            if (-1 == nids) return 0;
            continue;
        }
        /* similar to mbedtls_ecp_curve_info_from_name() */
        const mbedtls_ecp_curve_info *info;
        for (info = curve_info; info->tls_id != 0; ++info) {
            if (0 == os_strncmp(info->name, n, len) && info->name[len] == '\0')
                break;
        }
        if (info->tls_id == 0) {
            wpa_printf(MSG_ERROR, "MTLS: unrecognized curve: %.*s",(int)len,n);
            return 0;
        }

        nids = tls_mbedtls_append_curve(ids, nids, idsz, info->tls_id);
        if (-1 == nids) return 0;
    }

    /* mod_openssl configures "prime256v1" if curve list not specified,
     * but mbedtls provides a list of supported curves if not explicitly set */
    if (-1 == nids) return 1; /* empty list; no-op */

    ids[++nids] = 0; /* terminate list */
    ++nids;

    /* curves list must be persistent for lifetime of mbedtls_ssl_config */
    tls_conf->curves = os_malloc(nids * sizeof(uint16_t));
    if (tls_conf->curves == NULL)
        return 0;
    os_memcpy(tls_conf->curves, ids, nids * sizeof(uint16_t));

    mbedtls_ssl_conf_groups(&tls_conf->conf, tls_conf->curves);
    return 1;
}
#endif /* MBEDTLS_VERSION_NUMBER >= 0x03010000 */ /* mbedtls 3.1.0 */


/* data copied from lighttpd src/mod_mbedtls.c (BSD-3-Clause) */
static const int suite_AES_256_ephemeral[] = {
    /* All AES-256 ephemeral suites */
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CCM,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CCM,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CCM_8
};

/* data copied from lighttpd src/mod_mbedtls.c (BSD-3-Clause) */
static const int suite_AES_128_ephemeral[] = {
    /* All AES-128 ephemeral suites */
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CCM,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CCM_8
};

/* data copied from lighttpd src/mod_mbedtls.c (BSD-3-Clause) */
/* HIGH cipher list (mapped from openssl list to mbedtls) */
static const int suite_HIGH[] = {
    MBEDTLS_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CCM,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CCM,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CCM_8,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384,
    MBEDTLS_TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384,
    MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_ARIA_256_GCM_SHA384,
    MBEDTLS_TLS_ECDHE_RSA_WITH_ARIA_256_GCM_SHA384,
    MBEDTLS_TLS_DHE_RSA_WITH_ARIA_256_GCM_SHA384,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CCM,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8,
    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CCM_8,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA,
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_ARIA_128_GCM_SHA256,
    MBEDTLS_TLS_ECDHE_RSA_WITH_ARIA_128_GCM_SHA256,
    MBEDTLS_TLS_DHE_RSA_WITH_ARIA_128_GCM_SHA256,
    MBEDTLS_TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256,
    MBEDTLS_TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CCM,
    MBEDTLS_TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384,
    MBEDTLS_TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CCM_8,
    MBEDTLS_TLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CCM,
    MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CCM_8,
    MBEDTLS_TLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_RSA_WITH_AES_256_CCM,
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_AES_256_CCM_8,
    MBEDTLS_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_ARIA_256_GCM_SHA384,
    MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_128_CCM,
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_AES_128_CCM_8,
    MBEDTLS_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_ARIA_128_GCM_SHA256,
    MBEDTLS_TLS_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256,
    MBEDTLS_TLS_RSA_PSK_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_RSA_PSK_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_RSA_PSK_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384,
    MBEDTLS_TLS_RSA_PSK_WITH_ARIA_256_GCM_SHA384,
    MBEDTLS_TLS_RSA_PSK_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_RSA_PSK_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_RSA_PSK_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_RSA_PSK_WITH_ARIA_128_GCM_SHA256,
    MBEDTLS_TLS_PSK_WITH_CHACHA20_POLY1305_SHA256,
    MBEDTLS_TLS_PSK_WITH_AES_256_GCM_SHA384,
    MBEDTLS_TLS_PSK_WITH_AES_256_CCM,
    MBEDTLS_TLS_PSK_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_PSK_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384,
    MBEDTLS_TLS_PSK_WITH_AES_256_CCM_8,
    MBEDTLS_TLS_PSK_WITH_ARIA_256_GCM_SHA384,
    MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_PSK_WITH_AES_128_CCM,
    MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256,
    MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8,
    MBEDTLS_TLS_PSK_WITH_ARIA_128_GCM_SHA256
};


__attribute_noinline__
static int
tls_mbedtls_append_ciphersuite (int *ids, int nids, int idsz, const int *x, int xsz)
{
    if (xsz >= idsz - (nids + 1)) {
        emsg(MSG_ERROR, "error: too many ciphers during list expand");
        return -1;
    }

    for (int i = 0; i < xsz; ++i)
        ids[++nids] = x[i];

    return nids;
}


static int
tls_mbedtls_translate_ciphername(int id, char *buf, size_t buflen)
{
    const mbedtls_ssl_ciphersuite_t *info =
      mbedtls_ssl_ciphersuite_from_id(id);
    if (info == NULL)
        return 0;
    const char *name = mbedtls_ssl_ciphersuite_get_name(info);
    const size_t len = os_strlen(name);
    if (len == 7 && 0 == os_memcmp(name, "unknown", 7))
        return 0;
    if (len >= buflen)
        return 0;
    os_strlcpy(buf, name, buflen);

    /* attempt to translate mbedtls string to openssl string
     * (some heuristics; incomplete) */
    size_t i = 0, j = 0;
    if (buf[0] == 'T') {
        if (os_strncmp(buf, "TLS1-3-", 7) == 0) {
            buf[3] = '-';
            j = 4; /* remove "1-3" from "TLS1-3-" prefix */
            i = 7;
        }
        else if (os_strncmp(buf, "TLS-", 4) == 0)
            i = 4; /* remove "TLS-" prefix */
    }
    for (; buf[i]; ++i) {
        if (buf[i] == '-') {
            if (i >= 3) {
                if (0 == os_memcmp(buf+i-3, "AES", 3))
                    continue; /* "AES-" -> "AES" */
            }
            if (i >= 4) {
                if (0 == os_memcmp(buf+i-4, "WITH", 4)) {
                    j -= 4;   /* remove "WITH-" */
                    continue;
                }
            }
        }
        buf[j++] = buf[i];
    }
    buf[j] = '\0';

    return j;
}


__attribute_noinline__
static int
tls_mbedtls_set_ciphersuites(struct tls_conf *tls_conf, int *ids, int nids)
{
    /* ciphersuites list must be persistent for lifetime of mbedtls_ssl_config*/
    os_free(tls_conf->ciphersuites);
    tls_conf->ciphersuites = os_malloc(nids * sizeof(int));
    if (tls_conf->ciphersuites == NULL)
        return 0;
    os_memcpy(tls_conf->ciphersuites, ids, nids * sizeof(int));
    mbedtls_ssl_conf_ciphersuites(&tls_conf->conf, tls_conf->ciphersuites);
    return 1;
}


static int
tls_mbedtls_set_ciphers(struct tls_conf *tls_conf, const char *ciphers)
{
    char buf[64];
    int ids[512];
    int nids = -1;
    const int idsz = (int)(sizeof(ids)/sizeof(*ids)-1);
    const char *next;
    size_t blen, clen;
    do {
        next = os_strchr(ciphers, ':');
        clen = next ? (size_t)(next - ciphers) : os_strlen(ciphers);
        if (!clen)
            continue;

        /* special-case a select set of openssl group names for hwsim tests */
	/* (review; remove excess code if tests are not run for non-OpenSSL?) */
        if (clen == 9 && os_memcmp(ciphers, "SUITEB192", 9) == 0) {
            static int ssl_preset_suiteb192_ciphersuites[] = {
                MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
                0
            };
            return tls_mbedtls_set_ciphersuites(tls_conf,
                                                ssl_preset_suiteb192_ciphersuites,
                                                2);
        }
        if (clen == 9 && os_memcmp(ciphers, "SUITEB128", 9) == 0) {
            static int ssl_preset_suiteb128_ciphersuites[] = {
                MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
                0
            };
            return tls_mbedtls_set_ciphersuites(tls_conf,
                                                ssl_preset_suiteb128_ciphersuites,
                                                2);
        }
        if (clen == 7 && os_memcmp(ciphers, "DEFAULT", 7) == 0)
            continue;
        if (clen == 6 && os_memcmp(ciphers, "AES128", 6) == 0) {
            nids = tls_mbedtls_append_ciphersuite(ids, nids, idsz,
                     suite_AES_128_ephemeral,
                     (int)ARRAY_SIZE(suite_AES_128_ephemeral));
            if (nids == -1)
                return 0;
            continue;
        }
        if (clen == 6 && os_memcmp(ciphers, "AES256", 6) == 0) {
            nids = tls_mbedtls_append_ciphersuite(ids, nids, idsz,
                     suite_AES_256_ephemeral,
                     (int)ARRAY_SIZE(suite_AES_256_ephemeral));
            if (nids == -1)
                return 0;
            continue;
        }
        if (clen == 4 && os_memcmp(ciphers, "HIGH", 4) == 0) {
            nids = tls_mbedtls_append_ciphersuite(ids, nids, idsz, suite_HIGH,
                                                  (int)ARRAY_SIZE(suite_HIGH));
            if (nids == -1)
                return 0;
            continue;
        }
        /* ignore anonymous cipher group names (?not supported by mbedtls?) */
        if (clen == 4 && os_memcmp(ciphers, "!ADH", 4) == 0)
            continue;
        if (clen == 6 && os_memcmp(ciphers, "-aECDH", 6) == 0)
            continue;
        if (clen == 7 && os_memcmp(ciphers, "-aECDSA", 7) == 0)
            continue;

        /* attempt to match mbedtls cipher names
         * nb: does not support openssl group names or list manipulation syntax
         *   (alt: could copy almost 1200 lines (!!!) of lighttpd mod_mbedtls.c
         *    mod_mbedtls_ssl_conf_ciphersuites() to translate strings)
         * note: not efficient to rewrite list for each ciphers entry,
         *       but this code is expected to run only at startup
         */
        const int *list = mbedtls_ssl_list_ciphersuites();
        for (; *list; ++list) {
            blen = tls_mbedtls_translate_ciphername(*list,buf,sizeof(buf));
            if (!blen)
                continue;

            /* matching heuristics additional to translate_ciphername above */
            if (blen == clen+4) {
                char *cbc = os_strstr(buf, "CBC-");
                if (cbc) {
                    os_memmove(cbc, cbc+4, blen-(cbc+4-buf)+1); /*(w/ '\0')*/
                    blen -= 4;
                }
            }
            if (blen >= clen && os_memcmp(ciphers, buf, clen) == 0
                && (blen == clen
                    || (blen == clen+7 && os_memcmp(buf+clen, "-SHA256", 7)))) {
                if (1 >= idsz - (nids + 1)) {
                    emsg(MSG_ERROR,
                         "error: too many ciphers during list expand");
                    return 0;
                }
                ids[++nids] = *list;
                break;
            }
        }
        if (*list == 0) {
            wpa_printf(MSG_ERROR,
                       "MTLS: unrecognized cipher: %.*s", (int)clen, ciphers);
            return 0;
        }
    } while ((ciphers = next ? next+1 : NULL));

    if (-1 == nids) return 1; /* empty list; no-op */

    ids[++nids] = 0; /* terminate list */
    ++nids;

    return tls_mbedtls_set_ciphersuites(tls_conf, ids, nids);
}


__attribute_noinline__
static int tls_mbedtls_set_item(char **config_item, const char *item)
{
	os_free(*config_item);
	*config_item = NULL;
	return item ? (*config_item = os_strdup(item)) != NULL : 1;
}


static int tls_connection_set_subject_match(struct tls_conf *tls_conf,
                                            const struct tls_connection_params *params)
{
	int rc = 1;
	rc &= tls_mbedtls_set_item(&tls_conf->subject_match,
	                              params->subject_match);
	rc &= tls_mbedtls_set_item(&tls_conf->altsubject_match,
	                              params->altsubject_match);
	rc &= tls_mbedtls_set_item(&tls_conf->suffix_match,
	                              params->suffix_match);
	rc &= tls_mbedtls_set_item(&tls_conf->domain_match,
	                              params->domain_match);
	rc &= tls_mbedtls_set_item(&tls_conf->check_cert_subject,
	                              params->check_cert_subject);
	return rc;
}


/* duplicated in crypto_mbedtls.c:crypto_mbedtls_readfile()*/
__attribute_noinline__
static int tls_mbedtls_readfile(const char *path, u8 **buf, size_t *n)
{
  #if 0 /* #ifdef MBEDTLS_FS_IO */
	/*(includes +1 for '\0' needed by mbedtls PEM parsing funcs)*/
	if (mbedtls_pk_load_file(path, (unsigned char **)buf, n) != 0) {
		wpa_printf(MSG_ERROR, "error: mbedtls_pk_load_file %s", path);
		return -1;
	}
  #else
	/*(use os_readfile() so that we can use os_free()
	 *(if we use mbedtls_pk_load_file() above, macros prevent calling free()
	 * directly #if defined(OS_REJECT_C_LIB_FUNCTIONS) and calling os_free()
	 * on buf aborts in tests if buf not allocated via os_malloc())*/
	*buf = (u8 *)os_readfile(path, n);
	if (!*buf) {
		wpa_printf(MSG_ERROR, "error: os_readfile %s", path);
		return -1;
	}
	u8 *buf0 = os_realloc(*buf, *n+1);
	if (!buf0) {
		bin_clear_free(*buf, *n);
		*buf = NULL;
		return -1;
	}
	buf0[(*n)++] = '\0';
	*buf = buf0;
  #endif
	return 0;
}


static int tls_mbedtls_set_crl(struct tls_conf *tls_conf, const u8 *data, size_t len)
{
	/* do not use mbedtls_x509_crl_parse() on PEM unless it contains CRL */
	if (len && data[len-1] == '\0'
	    && NULL == os_strstr((const char *)data,"-----BEGIN X509 CRL-----")
	    && tls_mbedtls_data_is_pem(data))
		return 0;

	mbedtls_x509_crl crl;
	mbedtls_x509_crl_init(&crl);
	int rc = mbedtls_x509_crl_parse(&crl, data, len);
	if (rc < 0) {
		mbedtls_x509_crl_free(&crl);
		return rc == MBEDTLS_ERR_PEM_NO_HEADER_FOOTER_PRESENT ? 0 : rc;
	}

	mbedtls_x509_crl *crl_new = os_malloc(sizeof(crl));
	if (crl_new == NULL) {
		mbedtls_x509_crl_free(&crl);
		return MBEDTLS_ERR_X509_ALLOC_FAILED;
	}
	os_memcpy(crl_new, &crl, sizeof(crl));

	mbedtls_x509_crl *crl_old = tls_conf->crl;
	tls_conf->crl = crl_new;
	if (crl_old) {
		mbedtls_x509_crl_free(crl_old);
		os_free(crl_old);
	}
	return 0;
}


static int tls_mbedtls_set_ca(struct tls_conf *tls_conf, u8 *data, size_t len)
{
	/* load crt struct onto stack and then copy into tls_conf in
	 * order to preserve existing tls_conf value if error occurs
	 *
	 * hostapd is not threaded, or else should allocate memory and swap in
	 * pointer reduce race condition.  (If threaded, would also need to
	 * keep reference count of use to avoid freeing while still in use.) */

	mbedtls_x509_crt crt;
	mbedtls_x509_crt_init(&crt);
	int rc = mbedtls_x509_crt_parse(&crt, data, len);
	if (rc < 0) {
		mbedtls_x509_crt_free(&crt);
		return rc;
	}

	mbedtls_x509_crt_free(&tls_conf->ca_cert);
	os_memcpy(&tls_conf->ca_cert, &crt, sizeof(crt));
	return 0;
}


static int tls_mbedtls_set_ca_and_crl(struct tls_conf *tls_conf, const char *ca_cert_file)
{
	size_t len;
	u8 *data;
	if (tls_mbedtls_readfile(ca_cert_file, &data, &len))
		return -1;

	int rc;
	if (0 == (rc = tls_mbedtls_set_ca(tls_conf, data, len))
	    && (!tls_mbedtls_data_is_pem(data) /*skip parse for CRL if not PEM*/
	        || 0 == (rc = tls_mbedtls_set_crl(tls_conf, data, len)))) {
		mbedtls_ssl_conf_ca_chain(&tls_conf->conf,
		                          &tls_conf->ca_cert,
		                          tls_conf->crl);
	}
	else {
		elog(rc, __func__);
		emsg(MSG_ERROR, ca_cert_file);
	}

	forced_memzero(data, len);
	os_free(data);
	return rc;
}


static void tls_mbedtls_refresh_crl(void)
{
	/* check for CRL refresh
	 * continue even if error occurs; continue with previous cert, CRL */
	unsigned int crl_reload_interval = tls_ctx_global.crl_reload_interval;
	const char *ca_cert_file = tls_ctx_global.ca_cert_file;
	if (!crl_reload_interval || !ca_cert_file)
		return;

	struct os_reltime *previous = &tls_ctx_global.crl_reload_previous;
	struct os_reltime now;
	if (os_get_reltime(&now) != 0
	    || !os_reltime_expired(&now, previous, crl_reload_interval))
		return;

	/* Note: modifying global state is not thread-safe
	 *       if in use by existing connections
	 *
	 * src/utils/os.h does not provide a portable stat()
	 * or else it would be a good idea to check mtime and size,
	 * and avoid reloading if file has not changed */

	if (tls_mbedtls_set_ca_and_crl(tls_ctx_global.tls_conf, ca_cert_file) == 0)
		*previous = now;
}


static int tls_mbedtls_set_ca_cert(struct tls_conf *tls_conf,
				   const struct tls_connection_params *params)
{
	if (params->ca_cert) {
		if (os_strncmp(params->ca_cert, "probe://", 8) == 0) {
			tls_conf->ca_cert_probe = 1;
			tls_conf->has_ca_cert = 1;
			return 0;
		}

		if (os_strncmp(params->ca_cert, "hash://", 7) == 0) {
			const char *pos = params->ca_cert + 7;
			if (os_strncmp(pos, "server/sha256/", 14) != 0) {
				emsg(MSG_ERROR, "unsupported ca_cert hash value");
				return -1;
			}
			pos += 14;
			if (os_strlen(pos) != SHA256_DIGEST_LENGTH*2) {
				emsg(MSG_ERROR, "unexpected ca_cert hash length");
				return -1;
			}
			if (hexstr2bin(pos, tls_conf->ca_cert_hash,
			               SHA256_DIGEST_LENGTH) < 0) {
				emsg(MSG_ERROR, "invalid ca_cert hash value");
				return -1;
			}
			emsg(MSG_DEBUG, "checking only server certificate match");
			tls_conf->verify_depth0_only = 1;
			tls_conf->has_ca_cert = 1;
			return 0;
		}

		if (tls_mbedtls_set_ca_and_crl(tls_conf, params->ca_cert) != 0)
			return -1;
	}
	if (params->ca_cert_blob) {
		size_t len = params->ca_cert_blob_len;
		int is_pem = tls_mbedtls_data_is_pem(params->ca_cert_blob);
		if (len && params->ca_cert_blob[len-1] != '\0' && is_pem)
			++len; /*(include '\0' in len for PEM)*/
		int ret = mbedtls_x509_crt_parse(&tls_conf->ca_cert,
		                                 params->ca_cert_blob, len);
		if (ret != 0) {
			elog(ret, "mbedtls_x509_crt_parse");
			return -1;
		}
		if (is_pem) { /*(ca_cert_blob in DER format contains ca cert only)*/
			ret = tls_mbedtls_set_crl(tls_conf, params->ca_cert_blob, len);
			if (ret != 0) {
				elog(ret, "mbedtls_x509_crl_parse");
				return -1;
			}
		}
	}

	if (mbedtls_x509_time_is_future(&tls_conf->ca_cert.valid_from)
	    || mbedtls_x509_time_is_past(&tls_conf->ca_cert.valid_to)) {
		emsg(MSG_WARNING, "ca_cert expired or not yet valid");
		if (params->ca_cert)
			emsg(MSG_WARNING, params->ca_cert);
	}

	tls_conf->has_ca_cert = 1;
	return 0;
}


static int tls_mbedtls_set_certs(struct tls_conf *tls_conf,
				 const struct tls_connection_params *params)
{
	int ret;

	if (params->ca_cert || params->ca_cert_blob) {
		if (tls_mbedtls_set_ca_cert(tls_conf, params) != 0)
			return -1;
	}
	else if (params->ca_path) {
		emsg(MSG_INFO, "ca_path support not implemented");
		return -1;
	}

	if (!tls_conf->has_ca_cert)
		mbedtls_ssl_conf_authmode(&tls_conf->conf, MBEDTLS_SSL_VERIFY_NONE);
	else {
		/* Initial setting: REQUIRED for client, OPTIONAL for server
		 *   (see also tls_connection_set_verify()) */
		tls_conf->verify_peer = (tls_ctx_global.tls_conf == NULL);
		int authmode = tls_conf->verify_peer
		  ? MBEDTLS_SSL_VERIFY_REQUIRED
		  : MBEDTLS_SSL_VERIFY_OPTIONAL;
		mbedtls_ssl_conf_authmode(&tls_conf->conf, authmode);
		mbedtls_ssl_conf_ca_chain(&tls_conf->conf,
		                          &tls_conf->ca_cert,
		                          tls_conf->crl);

		if (!tls_connection_set_subject_match(tls_conf, params))
			return -1;
	}

	if (params->client_cert2) /*(yes, server_cert2 in msg below)*/
		emsg(MSG_INFO, "server_cert2 support not implemented");

	if (params->client_cert) {
		size_t len;
		u8 *data;
		if (tls_mbedtls_readfile(params->client_cert, &data, &len))
			return -1;
		ret = mbedtls_x509_crt_parse(&tls_conf->client_cert, data, len);
		forced_memzero(data, len);
		os_free(data);
	}
	if (params->client_cert_blob) {
		size_t len = params->client_cert_blob_len;
		if (len && params->client_cert_blob[len-1] != '\0'
		    && tls_mbedtls_data_is_pem(params->client_cert_blob))
			++len; /*(include '\0' in len for PEM)*/
		ret = mbedtls_x509_crt_parse(&tls_conf->client_cert,
		                             params->client_cert_blob, len);
	}
	if (params->client_cert || params->client_cert_blob) {
		if (ret < 0) {
			elog(ret, "mbedtls_x509_crt_parse");
			if (params->client_cert)
				emsg(MSG_ERROR, params->client_cert);
			return -1;
		}
		if (mbedtls_x509_time_is_future(&tls_conf->client_cert.valid_from)
		    || mbedtls_x509_time_is_past(&tls_conf->client_cert.valid_to)) {
			emsg(MSG_WARNING, "cert expired or not yet valid");
			if (params->client_cert)
				emsg(MSG_WARNING, params->client_cert);
		}
		tls_conf->has_client_cert = 1;
	}

	if (params->private_key || params->private_key_blob) {
		size_t len = params->private_key_blob_len;
		u8 *data;
		*(const u8 **)&data = params->private_key_blob;
		if (len && data[len-1] != '\0' && tls_mbedtls_data_is_pem(data))
			++len; /*(include '\0' in len for PEM)*/
		if (params->private_key
		    && tls_mbedtls_readfile(params->private_key, &data, &len)) {
			return -1;
		}
		const char *pwd = params->private_key_passwd;
	  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
		ret = mbedtls_pk_parse_key(&tls_conf->private_key,
			data, len,
			(const unsigned char *)pwd,
			pwd ? os_strlen(pwd) : 0,
			mbedtls_ctr_drbg_random,
			tls_ctx_global.ctr_drbg);
	  #else
		ret = mbedtls_pk_parse_key(&tls_conf->private_key,
			data, len,
			(const unsigned char *)pwd,
			pwd ? os_strlen(pwd) : 0);
	  #endif
		if (params->private_key) {
			forced_memzero(data, len);
			os_free(data);
		}
		if (ret < 0) {
			elog(ret, "mbedtls_pk_parse_key");
			return -1;
		}
		tls_conf->has_private_key = 1;
	}

	if (tls_conf->has_client_cert && tls_conf->has_private_key) {
		ret = mbedtls_ssl_conf_own_cert(
		    &tls_conf->conf, &tls_conf->client_cert, &tls_conf->private_key);
		if (ret < 0) {
			elog(ret, "mbedtls_ssl_conf_own_cert");
			return -1;
		}
	}

	return 0;
}


/* mbedtls_x509_crt_profile_suiteb plus rsa_min_bitlen 2048 */
/* (reference: see also mbedtls_x509_crt_profile_next) */
/* ??? should permit SHA-512, too, and additional curves ??? */
static const mbedtls_x509_crt_profile tls_mbedtls_crt_profile_suiteb128 =
{
    /* Only SHA-256 and 384 */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ),
    /* Only ECDSA */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_PK_ECDSA ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_PK_ECKEY ),
#if defined(MBEDTLS_ECP_C)
    /* Only NIST P-256 and P-384 */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_ECP_DP_SECP256R1 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_ECP_DP_SECP384R1 ),
#else
    0,
#endif
    2048,
};


/* stricter than mbedtls_x509_crt_profile_suiteb */
/* (reference: see also mbedtls_x509_crt_profile_next) */
/* ??? should permit SHA-512, too, and additional curves ??? */
static const mbedtls_x509_crt_profile tls_mbedtls_crt_profile_suiteb192 =
{
    /* Only SHA-384 */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ),
    /* Only ECDSA */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_PK_ECDSA ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_PK_ECKEY ),
#if defined(MBEDTLS_ECP_C)
    /* Only NIST P-384 */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_ECP_DP_SECP384R1 ),
#else
    0,
#endif
    3072,
};


/* stricter than mbedtls_x509_crt_profile_suiteb except allow any PK alg */
/* (reference: see also mbedtls_x509_crt_profile_next) */
/* ??? should permit SHA-512, too, and additional curves ??? */
static const mbedtls_x509_crt_profile tls_mbedtls_crt_profile_suiteb192_anypk =
{
    /* Only SHA-384 */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ),
    0xFFFFFFF, /* Any PK alg    */
#if defined(MBEDTLS_ECP_C)
    /* Only NIST P-384 */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_ECP_DP_SECP384R1 ),
#else
    0,
#endif
    3072,
};


static int tls_mbedtls_set_params(struct tls_conf *tls_conf,
				  const struct tls_connection_params *params)
{
	tls_conf->flags = params->flags;

	if (tls_conf->flags & TLS_CONN_REQUIRE_OCSP_ALL) {
		emsg(MSG_INFO, "ocsp=3 not supported");
		return -1;
	}

	if (tls_conf->flags & TLS_CONN_REQUIRE_OCSP) {
		emsg(MSG_INFO, "ocsp not supported");
		return -1;
	}

	int suiteb128 = 0;
	int suiteb192 = 0;
	if (params->openssl_ciphers) {
		if (os_strcmp(params->openssl_ciphers, "SUITEB192") == 0) {
			suiteb192 = 1;
			tls_conf->flags |= TLS_CONN_SUITEB;
		}
		if (os_strcmp(params->openssl_ciphers, "SUITEB128") == 0) {
			suiteb128 = 1;
			tls_conf->flags |= TLS_CONN_SUITEB;
		}
	}

	int ret = mbedtls_ssl_config_defaults(
	    &tls_conf->conf, tls_ctx_global.tls_conf ? MBEDTLS_SSL_IS_SERVER
	                                             : MBEDTLS_SSL_IS_CLIENT,
	    MBEDTLS_SSL_TRANSPORT_STREAM,
	    (tls_conf->flags & TLS_CONN_SUITEB) ? MBEDTLS_SSL_PRESET_SUITEB
	                                        : MBEDTLS_SSL_PRESET_DEFAULT);
	if (ret != 0) {
		elog(ret, "mbedtls_ssl_config_defaults");
		return -1;
	}

	if (suiteb128) {
		mbedtls_ssl_conf_cert_profile(&tls_conf->conf,
		                              &tls_mbedtls_crt_profile_suiteb128);
		mbedtls_ssl_conf_dhm_min_bitlen(&tls_conf->conf, 2048);
	}
	else if (suiteb192) {
		mbedtls_ssl_conf_cert_profile(&tls_conf->conf,
		                              &tls_mbedtls_crt_profile_suiteb192);
		mbedtls_ssl_conf_dhm_min_bitlen(&tls_conf->conf, 3072);
	}
	else if (tls_conf->flags & TLS_CONN_SUITEB) {
		/* treat as suiteb192 while allowing any PK algorithm */
		mbedtls_ssl_conf_cert_profile(&tls_conf->conf,
		                              &tls_mbedtls_crt_profile_suiteb192_anypk);
		mbedtls_ssl_conf_dhm_min_bitlen(&tls_conf->conf, 3072);
	}

	tls_mbedtls_set_allowed_tls_vers(tls_conf, &tls_conf->conf);
	ret = tls_mbedtls_set_certs(tls_conf, params);
	if (ret != 0)
		return -1;

	if (params->dh_file
	    && !tls_mbedtls_set_dhparams(tls_conf, params->dh_file)) {
		return -1;
	}

	if (params->openssl_ecdh_curves
	    && !tls_mbedtls_set_curves(tls_conf, params->openssl_ecdh_curves)) {
		return -1;
	}

	if (params->openssl_ciphers) {
		if (!tls_mbedtls_set_ciphers(tls_conf, params->openssl_ciphers))
			return -1;
	}
	else if (tls_conf->flags & TLS_CONN_SUITEB) {
		/* special-case a select set of ciphers for hwsim tests */
		if (!tls_mbedtls_set_ciphers(tls_conf,
		        (tls_conf->flags & TLS_CONN_SUITEB_NO_ECDH)
		          ? "DHE-RSA-AES256-GCM-SHA384"
		          : "ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384"))
			return -1;
	}

	return 0;
}


int tls_connection_set_params(void *tls_ctx, struct tls_connection *conn,
			      const struct tls_connection_params *params)
{
	if (conn == NULL || params == NULL)
		return -1;

	tls_conf_deinit(conn->tls_conf);
	struct tls_conf *tls_conf = conn->tls_conf = tls_conf_init(tls_ctx);
	if (tls_conf == NULL)
		return -1;

	if (tls_ctx_global.tls_conf) {
		tls_conf->check_crl = tls_ctx_global.tls_conf->check_crl;
		tls_conf->check_crl_strict = tls_ctx_global.tls_conf->check_crl_strict;
		/*(tls_openssl.c inherits check_cert_subject from global conf)*/
		if (tls_ctx_global.tls_conf->check_cert_subject) {
			tls_conf->check_cert_subject =
			  os_strdup(tls_ctx_global.tls_conf->check_cert_subject);
			if (tls_conf->check_cert_subject == NULL)
				return -1;
		}
	}

	if (tls_mbedtls_set_params(tls_conf, params) != 0)
		return -1;
	conn->verify_peer = tls_conf->verify_peer;

	return tls_mbedtls_ssl_setup(conn);
}


#ifdef TLS_MBEDTLS_SESSION_TICKETS

static int tls_mbedtls_clienthello_session_ticket_prep (struct tls_connection *conn,
                                                        const u8 *data, size_t len)
{
	if (conn->tls_conf->flags & TLS_CONN_DISABLE_SESSION_TICKET)
		return -1;
	if (conn->clienthello_session_ticket)
		tls_connection_deinit_clienthello_session_ticket(conn);
	if (len) {
		conn->clienthello_session_ticket = mbedtls_calloc(1, len);
		if (conn->clienthello_session_ticket == NULL)
			return -1;
		conn->clienthello_session_ticket_len = len;
		os_memcpy(conn->clienthello_session_ticket, data, len);
	}
	return 0;
}


static void tls_mbedtls_clienthello_session_ticket_set (struct tls_connection *conn)
{
	mbedtls_ssl_session *sess = conn->ssl.MBEDTLS_PRIVATE(session_negotiate);
	if (sess->MBEDTLS_PRIVATE(ticket)) {
		mbedtls_platform_zeroize(sess->MBEDTLS_PRIVATE(ticket),
		                         sess->MBEDTLS_PRIVATE(ticket_len));
		mbedtls_free(sess->MBEDTLS_PRIVATE(ticket));
	}
	sess->MBEDTLS_PRIVATE(ticket) = conn->clienthello_session_ticket;
	sess->MBEDTLS_PRIVATE(ticket_len) = conn->clienthello_session_ticket_len;
	sess->MBEDTLS_PRIVATE(ticket_lifetime) = 86400;/* XXX: can hint be 0? */

	conn->clienthello_session_ticket = NULL;
	conn->clienthello_session_ticket_len = 0;
}


static int tls_mbedtls_ssl_ticket_write(void *p_ticket,
                                        const mbedtls_ssl_session *session,
                                        unsigned char *start,
                                        const unsigned char *end,
                                        size_t *tlen,
                                        uint32_t *lifetime)
{
	struct tls_connection *conn = p_ticket;
	if (conn && conn->session_ticket_cb) {
		/* see tls_mbedtls_clienthello_session_ticket_prep() */
		/* see tls_mbedtls_clienthello_session_ticket_set() */
		return 0;
	}

	return mbedtls_ssl_ticket_write(&tls_ctx_global.ticket_ctx,
	                                session, start, end, tlen, lifetime);
}


static int tls_mbedtls_ssl_ticket_parse(void *p_ticket,
                                        mbedtls_ssl_session *session,
                                        unsigned char *buf,
                                        size_t len)
{
	/* XXX: TODO: not implemented in client;
	 * mbedtls_ssl_conf_session_tickets_cb() callbacks only for TLS server*/

	if (len == 0)
		return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;

	struct tls_connection *conn = p_ticket;
	if (conn && conn->session_ticket_cb) {
		/* XXX: have random and secret been initialized yet?
		 *      or must keys first be exported?
		 *      EAP-FAST uses all args, EAP-TEAP only uses secret */
		struct tls_random data;
		if (tls_connection_get_random(NULL, conn, &data) != 0)
			return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
		int ret =
		  conn->session_ticket_cb(conn->session_ticket_cb_ctx,
		                          buf, len,
		                          data.client_random,
		                          data.server_random,
		                          conn->expkey_secret);
		if (ret == 1) {
			conn->resumed = 1;
			return 0;
		}
		emsg(MSG_ERROR, "EAP session ticket ext not implemented");
		return MBEDTLS_ERR_SSL_INVALID_MAC;
		/*(non-zero return used for mbedtls debug logging)*/
	}

	/* XXX: TODO always use tls_mbedtls_ssl_ticket_parse() for callback? */
	int rc = mbedtls_ssl_ticket_parse(&tls_ctx_global.ticket_ctx,
	                                  session, buf, len);
	if (conn)
		conn->resumed = (rc == 0);
	return rc;
}

#endif /* TLS_MBEDTLS_SESSION_TICKETS */


__attribute_cold__
int tls_global_set_params(void *tls_ctx,
			  const struct tls_connection_params *params)
{
	/* XXX: why might global_set_params be called more than once? */
	if (tls_ctx_global.tls_conf)
		tls_conf_deinit(tls_ctx_global.tls_conf);
	tls_ctx_global.tls_conf = tls_conf_init(tls_ctx);
	if (tls_ctx_global.tls_conf == NULL)
		return -1;

  #ifdef MBEDTLS_SSL_SESSION_TICKETS
  #ifdef MBEDTLS_SSL_TICKET_C
	if (!(params->flags & TLS_CONN_DISABLE_SESSION_TICKET))
	  #ifdef TLS_MBEDTLS_SESSION_TICKETS
		mbedtls_ssl_conf_session_tickets_cb(&tls_ctx_global.tls_conf->conf,
		                                    tls_mbedtls_ssl_ticket_write,
		                                    tls_mbedtls_ssl_ticket_parse,
		                                    NULL);
	  #else
		mbedtls_ssl_conf_session_tickets_cb(&tls_ctx_global.tls_conf->conf,
		                                    mbedtls_ssl_ticket_write,
		                                    mbedtls_ssl_ticket_parse,
		                                    &tls_ctx_global.ticket_ctx);
	  #endif
  #endif
  #endif

	os_free(tls_ctx_global.ocsp_stapling_response);
	tls_ctx_global.ocsp_stapling_response = NULL;
	if (params->ocsp_stapling_response)
		tls_ctx_global.ocsp_stapling_response =
			os_strdup(params->ocsp_stapling_response);

	os_free(tls_ctx_global.ca_cert_file);
	tls_ctx_global.ca_cert_file = NULL;
	if (params->ca_cert)
		tls_ctx_global.ca_cert_file = os_strdup(params->ca_cert);
	return tls_mbedtls_set_params(tls_ctx_global.tls_conf, params);
}


int tls_global_set_verify(void *tls_ctx, int check_crl, int strict)
{
	tls_ctx_global.tls_conf->check_crl = check_crl;
	tls_ctx_global.tls_conf->check_crl_strict = strict; /*(time checks)*/
	return 0;
}


int tls_connection_set_verify(void *tls_ctx, struct tls_connection *conn,
			      int verify_peer, unsigned int flags,
			      const u8 *session_ctx, size_t session_ctx_len)
{
	/*(EAP server-side calls this from eap_server_tls_ssl_init())*/
	if (conn == NULL)
		return -1;

	conn->tls_conf->flags |= flags;/* TODO: reprocess flags, if necessary */

	int authmode;
	switch (verify_peer) {
	case 2:  authmode = MBEDTLS_SSL_VERIFY_OPTIONAL; break;/*(eap_teap_init())*/
	case 1:  authmode = MBEDTLS_SSL_VERIFY_REQUIRED; break;
	default: authmode = MBEDTLS_SSL_VERIFY_NONE;     break;
	}
	mbedtls_ssl_set_hs_authmode(&conn->ssl, authmode);

	if ((conn->verify_peer = (authmode != MBEDTLS_SSL_VERIFY_NONE)))
		mbedtls_ssl_set_verify(&conn->ssl, tls_mbedtls_verify_cb, conn);
	else
		mbedtls_ssl_set_verify(&conn->ssl, NULL, NULL);

	return 0;
}


#if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
static void tls_connection_export_keys_cb(
    void *p_expkey, mbedtls_ssl_key_export_type secret_type,
    const unsigned char *secret, size_t secret_len,
    const unsigned char client_random[MBEDTLS_EXPKEY_RAND_LEN],
    const unsigned char server_random[MBEDTLS_EXPKEY_RAND_LEN],
    mbedtls_tls_prf_types tls_prf_type)
{
	struct tls_connection *conn = p_expkey;
	conn->tls_prf_type = tls_prf_type;
	if (!tls_prf_type)
		return;
	if (secret_len > sizeof(conn->expkey_secret)) {
		emsg(MSG_ERROR, "tls_connection_export_keys_cb secret too long");
		conn->tls_prf_type = MBEDTLS_SSL_TLS_PRF_NONE; /* 0 */
		return;
	}
	conn->expkey_secret_len = secret_len;
	os_memcpy(conn->expkey_secret, secret, secret_len);
	os_memcpy(conn->expkey_randbytes,
	          client_random, MBEDTLS_EXPKEY_RAND_LEN);
	os_memcpy(conn->expkey_randbytes + MBEDTLS_EXPKEY_RAND_LEN,
	          server_random, MBEDTLS_EXPKEY_RAND_LEN);
}
#elif MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */
static int tls_connection_export_keys_cb(
    void *p_expkey,
    const unsigned char *ms,
    const unsigned char *kb,
    size_t maclen,
    size_t keylen,
    size_t ivlen,
    const unsigned char client_random[MBEDTLS_EXPKEY_RAND_LEN],
    const unsigned char server_random[MBEDTLS_EXPKEY_RAND_LEN],
    mbedtls_tls_prf_types tls_prf_type )
{
	struct tls_connection *conn = p_expkey;
	conn->tls_prf_type = tls_prf_type;
	if (!tls_prf_type)
		return -1; /*(return value ignored by mbedtls)*/
	conn->expkey_keyblock_size = maclen + keylen + ivlen;
	conn->expkey_secret_len = MBEDTLS_EXPKEY_FIXED_SECRET_LEN;
	os_memcpy(conn->expkey_secret, ms, MBEDTLS_EXPKEY_FIXED_SECRET_LEN);
	os_memcpy(conn->expkey_randbytes,
	          client_random, MBEDTLS_EXPKEY_RAND_LEN);
	os_memcpy(conn->expkey_randbytes + MBEDTLS_EXPKEY_RAND_LEN,
	          server_random, MBEDTLS_EXPKEY_RAND_LEN);
	return 0;
}
#endif


int tls_connection_get_random(void *tls_ctx, struct tls_connection *conn,
			      struct tls_random *data)
{
	if (!conn || !conn->tls_prf_type)
		return -1;
	data->client_random = conn->expkey_randbytes;
	data->client_random_len = MBEDTLS_EXPKEY_RAND_LEN;
	data->server_random = conn->expkey_randbytes + MBEDTLS_EXPKEY_RAND_LEN;
	data->server_random_len = MBEDTLS_EXPKEY_RAND_LEN;
	return 0;
}


int tls_connection_export_key(void *tls_ctx, struct tls_connection *conn,
			      const char *label, const u8 *context,
			      size_t context_len, u8 *out, size_t out_len)
{
	/* (EAP-PEAP EAP-TLS EAP-TTLS) */
  #if MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */
	return (conn && conn->established && conn->tls_prf_type)
	  ? mbedtls_ssl_tls_prf(conn->tls_prf_type,
				conn->expkey_secret, conn->expkey_secret_len, label,
				conn->expkey_randbytes,
				sizeof(conn->expkey_randbytes), out, out_len)
	  : -1;
  #else
	/* not implemented here for mbedtls < 2.18.0 */
	return -1;
  #endif
}


#ifdef TLS_MBEDTLS_EAP_FAST

#if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
/* keyblock size info is not exposed in mbed TLS 3.0.0 */
/* extracted from mbedtls library/ssl_tls.c:ssl_tls12_populate_transform() */
#include <mbedtls/ssl_ciphersuites.h>
#include <mbedtls/cipher.h>
static size_t tls_mbedtls_ssl_keyblock_size (mbedtls_ssl_context *ssl)
{
  #if !defined(MBEDTLS_USE_PSA_CRYPTO) /* XXX: (not extracted for PSA crypto) */
  #if defined(MBEDTLS_SSL_PROTO_TLS1_3)
    if (tls_version == MBEDTLS_SSL_VERSION_TLS1_3)
        return 0; /* (calculation not extracted) */
  #endif /* MBEDTLS_SSL_PROTO_TLS1_3 */

    int ciphersuite = mbedtls_ssl_get_ciphersuite_id_from_ssl(ssl);
    const mbedtls_ssl_ciphersuite_t *ciphersuite_info =
      mbedtls_ssl_ciphersuite_from_id(ciphersuite);
    if (ciphersuite_info == NULL)
        return 0;

    const mbedtls_cipher_info_t *cipher_info =
      mbedtls_cipher_info_from_type(ciphersuite_info->MBEDTLS_PRIVATE(cipher));
    if (cipher_info == NULL)
        return 0;

  #if MBEDTLS_VERSION_NUMBER >= 0x03010000 /* mbedtls 3.1.0 */
    size_t keylen = mbedtls_cipher_info_get_key_bitlen(cipher_info) / 8;
    mbedtls_cipher_mode_t mode = mbedtls_cipher_info_get_mode(cipher_info);
  #else
    size_t keylen = cipher_info->MBEDTLS_PRIVATE(key_bitlen) / 8;
    mbedtls_cipher_mode_t mode = cipher_info->MBEDTLS_PRIVATE(mode);
  #endif
  #if defined(MBEDTLS_GCM_C) || \
      defined(MBEDTLS_CCM_C) || \
      defined(MBEDTLS_CHACHAPOLY_C)
    if (mode == MBEDTLS_MODE_GCM || mode == MBEDTLS_MODE_CCM)
        return keylen + 4;
    else if (mode == MBEDTLS_MODE_CHACHAPOLY)
        return keylen + 12;
    else
  #endif /* MBEDTLS_GCM_C || MBEDTLS_CCM_C || MBEDTLS_CHACHAPOLY_C */
  #if defined(MBEDTLS_SSL_SOME_SUITES_USE_MAC)
    {
        const mbedtls_md_info_t *md_info =
          mbedtls_md_info_from_type(ciphersuite_info->MBEDTLS_PRIVATE(mac));
        if (md_info == NULL)
            return 0;
        size_t mac_key_len = mbedtls_md_get_size(md_info);
        size_t ivlen = mbedtls_cipher_info_get_iv_size(cipher_info);
        return keylen + mac_key_len + ivlen;
    }
  #endif /* MBEDTLS_SSL_SOME_SUITES_USE_MAC */
  #endif /* !MBEDTLS_USE_PSA_CRYPTO *//* (not extracted for PSA crypto) */
    return 0;
}
#endif /* MBEDTLS_VERSION_NUMBER >= 0x03000000 *//* mbedtls 3.0.0 */


int tls_connection_get_eap_fast_key(void *tls_ctx, struct tls_connection *conn,
				    u8 *out, size_t out_len)
{
	/* XXX: has export keys callback been run? */
	if (!conn || !conn->tls_prf_type)
		return -1;

  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	conn->expkey_keyblock_size = tls_mbedtls_ssl_keyblock_size(&conn->ssl);
	if (conn->expkey_keyblock_size == 0)
		return -1;
  #endif
	size_t skip = conn->expkey_keyblock_size * 2;
	unsigned char *tmp_out = os_malloc(skip + out_len);
	if (!tmp_out)
		return -1;

	/* server_random and then client_random */
	unsigned char seed[MBEDTLS_EXPKEY_RAND_LEN*2];
	os_memcpy(seed, conn->expkey_randbytes + MBEDTLS_EXPKEY_RAND_LEN,
	          MBEDTLS_EXPKEY_RAND_LEN);
	os_memcpy(seed + MBEDTLS_EXPKEY_RAND_LEN, conn->expkey_randbytes,
	          MBEDTLS_EXPKEY_RAND_LEN);

  #if MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */
	int ret = mbedtls_ssl_tls_prf(conn->tls_prf_type,
				      conn->expkey_secret, conn->expkey_secret_len,
				      "key expansion", seed, sizeof(seed),
				      tmp_out, skip + out_len);
	if (ret == 0)
		os_memcpy(out, tmp_out + skip, out_len);
  #else
	int ret = -1; /*(not reached if not impl; return -1 at top of func)*/
  #endif

	bin_clear_free(tmp_out, skip + out_len);
	forced_memzero(seed, sizeof(seed));
	return ret;
}

#endif /* TLS_MBEDTLS_EAP_FAST */


__attribute_cold__
static void tls_mbedtls_suiteb_handshake_alert (struct tls_connection *conn)
{
	/* tests/hwsim/test_suite_b.py test_suite_b_192_rsa_insufficient_dh */
	if (!(conn->tls_conf->flags & TLS_CONN_SUITEB))
		return;
	if (tls_ctx_global.tls_conf) /*(is server; want issue event on client)*/
		return;
  #if 0
	/*(info not available on client;
         * mbed TLS library enforces dhm min bitlen in ServerKeyExchange)*/
	if (MBEDTLS_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384 ==
	  #if MBEDTLS_VERSION_NUMBER < 0x03020000 /* mbedtls 3.2.0 */
	          mbedtls_ssl_get_ciphersuite_id_from_ssl(&conn->ssl)
	  #else
	          mbedtls_ssl_get_ciphersuite_id(
	            mbedtls_ssl_get_ciphersuite(&conn->ssl))
	  #endif
	    && mbedtls_mpi_size(&conn->tls_conf->conf.MBEDTLS_PRIVATE(dhm_P))
	         < 384 /*(3072/8)*/)
  #endif
	{
		struct tls_config *init_conf = &tls_ctx_global.init_conf;
		if (init_conf->event_cb) {
			union tls_event_data ev;
			os_memset(&ev, 0, sizeof(ev));
			ev.alert.is_local = 1;
			ev.alert.type = "fatal";
			/*"internal error" string for tests/hwsim/test_suiteb.py */
			ev.alert.description = "internal error: handshake failure";
			/*ev.alert.description = "insufficient security";*/
			init_conf->event_cb(init_conf->cb_ctx, TLS_ALERT, &ev);
		}
	}
}


struct wpabuf * tls_connection_handshake(void *tls_ctx,
					 struct tls_connection *conn,
					 const struct wpabuf *in_data,
					 struct wpabuf **appl_data)
{
	if (appl_data)
		*appl_data = NULL;

	if (in_data && wpabuf_len(in_data)) {
		/*(unsure why tls_gnutls.c discards buffer contents; skip here)*/
		if (conn->pull_buf && 0) /* disable; appears unwise */
			tls_pull_buf_discard(conn, __func__);
		if (!tls_pull_buf_append(conn, in_data))
			return NULL;
	}

	if (conn->tls_conf == NULL) {
		struct tls_connection_params params;
		os_memset(&params, 0, sizeof(params));
		params.openssl_ciphers =
		  tls_ctx_global.init_conf.openssl_ciphers;
		params.flags = tls_ctx_global.tls_conf->flags;
		if (tls_connection_set_params(tls_ctx, conn, &params) != 0)
			return NULL;
	}

	if (conn->verify_peer) /*(call here might be redundant; nbd)*/
		mbedtls_ssl_set_verify(&conn->ssl, tls_mbedtls_verify_cb, conn);

  #ifdef TLS_MBEDTLS_SESSION_TICKETS
	if (conn->clienthello_session_ticket)
		/*(starting handshake for EAP-FAST and EAP-TEAP)*/
		tls_mbedtls_clienthello_session_ticket_set(conn);

	/* (not thread-safe due to need to set userdata 'conn' for callback) */
	/* (unable to use mbedtls_ssl_set_user_data_p() with mbedtls 3.2.0+
	 *  since ticket write and parse callbacks take (mbedtls_ssl_session *)
	 *  param instead of (mbedtls_ssl_context *) param) */
	if (conn->tls_conf->flags & TLS_CONN_DISABLE_SESSION_TICKET)
		mbedtls_ssl_conf_session_tickets_cb(&conn->tls_conf->conf,
		                                    NULL, NULL, NULL);
	else
		mbedtls_ssl_conf_session_tickets_cb(&conn->tls_conf->conf,
		                                    tls_mbedtls_ssl_ticket_write,
		                                    tls_mbedtls_ssl_ticket_parse,
		                                    conn);
  #endif

  #if MBEDTLS_VERSION_NUMBER >= 0x03020000 /* mbedtls 3.2.0 */
	int ret = mbedtls_ssl_handshake(&conn->ssl);
  #else
	int ret = 0;
	while (conn->ssl.MBEDTLS_PRIVATE(state) != MBEDTLS_SSL_HANDSHAKE_OVER) {
		ret = mbedtls_ssl_handshake_step(&conn->ssl);
		if (ret != 0)
			break;
	}
  #endif

  #ifdef TLS_MBEDTLS_SESSION_TICKETS
	mbedtls_ssl_conf_session_tickets_cb(&conn->tls_conf->conf,
	                                    tls_mbedtls_ssl_ticket_write,
	                                    tls_mbedtls_ssl_ticket_parse,
	                                    NULL);
  #endif

	switch (ret) {
	case 0:
		conn->established = 1;
		if (conn->push_buf == NULL)
			/* Need to return something to get final TLS ACK. */
			conn->push_buf = wpabuf_alloc(0);

		if (appl_data /*&& conn->pull_buf && wpabuf_len(conn->pull_buf)*/)
			*appl_data = NULL; /* RFE: check for application data */
		break;
	case MBEDTLS_ERR_SSL_WANT_WRITE:
	case MBEDTLS_ERR_SSL_WANT_READ:
	case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
	case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
		if (tls_ctx_global.tls_conf /*(is server)*/
		    && conn->established && conn->push_buf == NULL)
			/* Need to return something to trigger completion of EAP-TLS. */
			conn->push_buf = wpabuf_alloc(0);
		break;
	default:
		++conn->failed;
		switch (ret) {
		case MBEDTLS_ERR_SSL_CLIENT_RECONNECT:
		case MBEDTLS_ERR_NET_CONN_RESET:
		case MBEDTLS_ERR_NET_SEND_FAILED:
			++conn->write_alerts;
			break;
	      #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
		case MBEDTLS_ERR_SSL_HANDSHAKE_FAILURE:
	      #else
		case MBEDTLS_ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE:
	      #endif
			tls_mbedtls_suiteb_handshake_alert(conn);
			/* fall through */
		case MBEDTLS_ERR_NET_RECV_FAILED:
		case MBEDTLS_ERR_SSL_CONN_EOF:
		case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
		case MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE:
			++conn->read_alerts;
			break;
		default:
			break;
		}

		ilog(ret, "mbedtls_ssl_handshake");
		break;
	}

	struct wpabuf *out_data = conn->push_buf;
	conn->push_buf = NULL;
	return out_data;
}


struct wpabuf * tls_connection_server_handshake(void *tls_ctx,
						struct tls_connection *conn,
						const struct wpabuf *in_data,
						struct wpabuf **appl_data)
{
	conn->is_server = 1;
	return tls_connection_handshake(tls_ctx, conn, in_data, appl_data);
}


struct wpabuf * tls_connection_encrypt(void *tls_ctx,
				       struct tls_connection *conn,
				       const struct wpabuf *in_data)
{
	int res = mbedtls_ssl_write(&conn->ssl,
	                            wpabuf_head_u8(in_data), wpabuf_len(in_data));
	if (res < 0) {
		elog(res, "mbedtls_ssl_write");
		return NULL;
	}

	struct wpabuf *buf = conn->push_buf;
	conn->push_buf = NULL;
	return buf;
}


struct wpabuf * tls_connection_decrypt(void *tls_ctx,
				       struct tls_connection *conn,
				       const struct wpabuf *in_data)
{
	int res;
	struct wpabuf *out;

	/*assert(in_data != NULL);*/
	if (!tls_pull_buf_append(conn, in_data))
		return NULL;

  #if defined(MBEDTLS_ZLIB_SUPPORT) /* removed in mbedtls 3.x */
	/* Add extra buffer space to handle the possibility of decrypted
	 * data being longer than input data due to TLS compression. */
	out = wpabuf_alloc((wpabuf_len(in_data) + 500) * 3);
  #else /* TLS compression is disabled in mbedtls 3.x */
	out = wpabuf_alloc(wpabuf_len(in_data));
  #endif
	if (out == NULL)
		return NULL;

	res = mbedtls_ssl_read(&conn->ssl, wpabuf_mhead(out), wpabuf_size(out));
	if (res < 0) {
	  #if 1 /*(seems like a different error if wpabuf_len(in_data) == 0)*/
		if (res == MBEDTLS_ERR_SSL_WANT_READ)
			return out;
	  #endif
		elog(res, "mbedtls_ssl_read");
		wpabuf_free(out);
		return NULL;
	}
	wpabuf_put(out, res);

	return out;
}


int tls_connection_resumed(void *tls_ctx, struct tls_connection *conn)
{
	/* XXX: might need to detect if session resumed from TLS session ticket
	 * even if not special session ticket handling for EAP-FAST, EAP-TEAP */
	/* (?ssl->handshake->resume during session ticket validation?) */
	return conn && conn->resumed;
}


#ifdef TLS_MBEDTLS_EAP_FAST
int tls_connection_set_cipher_list(void *tls_ctx, struct tls_connection *conn,
				   u8 *ciphers)
{
	/* ciphers is list of TLS_CIPHER_* from hostap/src/crypto/tls.h */
	int ids[7];
	const int idsz = (int)sizeof(ids);
	int nids = -1, id;
	for ( ; *ciphers != TLS_CIPHER_NONE; ++ciphers) {
		switch (*ciphers) {
		case TLS_CIPHER_RC4_SHA:
		  #ifdef MBEDTLS_TLS_RSA_WITH_RC4_128_SHA
			id = MBEDTLS_TLS_RSA_WITH_RC4_128_SHA;
			break;
		  #else
			continue; /*(not supported in mbedtls 3.x; ignore)*/
		  #endif
		case TLS_CIPHER_AES128_SHA:
			id = MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA;
			break;
		case TLS_CIPHER_RSA_DHE_AES128_SHA:
			id = MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CBC_SHA;
			break;
		case TLS_CIPHER_ANON_DH_AES128_SHA:
			continue; /*(not supported in mbedtls; ignore)*/
		case TLS_CIPHER_RSA_DHE_AES256_SHA:
			id = MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CBC_SHA;
			break;
		case TLS_CIPHER_AES256_SHA:
			id = MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA;
			break;
		default:
			return -1; /* should not happen */
		}
		if (++nids == idsz)
			return -1; /* should not happen */
		ids[nids] = id;
	}
	if (nids < 0)
		return 0; /* nothing to do */
	if (++nids == idsz)
		return -1; /* should not happen */
	ids[nids] = 0; /* terminate list */
	++nids;

	return tls_mbedtls_set_ciphersuites(conn->tls_conf, ids, nids) ? 0 : -1;
}
#endif


int tls_get_version(void *ssl_ctx, struct tls_connection *conn,
		    char *buf, size_t buflen)
{
	if (conn == NULL)
		return -1;
	os_strlcpy(buf, mbedtls_ssl_get_version(&conn->ssl), buflen);
	return buf[0] != 'u' ? 0 : -1; /*(-1 if "unknown")*/
}


#ifdef TLS_MBEDTLS_EAP_TEAP
u16 tls_connection_get_cipher_suite(struct tls_connection *conn)
{
	if (conn == NULL)
		return 0;
	return (u16)mbedtls_ssl_get_ciphersuite_id_from_ssl(&conn->ssl);
}
#endif


int tls_get_cipher(void *tls_ctx, struct tls_connection *conn,
		   char *buf, size_t buflen)
{
	if (conn == NULL)
		return -1;
	const int id = mbedtls_ssl_get_ciphersuite_id_from_ssl(&conn->ssl);
	return tls_mbedtls_translate_ciphername(id, buf, buflen) ? 0 : -1;
}


#ifdef TLS_MBEDTLS_SESSION_TICKETS

int tls_connection_enable_workaround(void *tls_ctx,
				     struct tls_connection *conn)
{
	/* (see comment in src/eap_peer/eap_fast.c:eap_fast_init()) */
	/* XXX: is there a relevant setting for this in mbed TLS? */
	/* (do we even care that much about older CBC ciphers?) */
	return 0;
}


int tls_connection_client_hello_ext(void *tls_ctx, struct tls_connection *conn,
				    int ext_type, const u8 *data,
				    size_t data_len)
{
	/* (EAP-FAST and EAP-TEAP) */
	if (ext_type == MBEDTLS_TLS_EXT_SESSION_TICKET) /*(ext_type == 35)*/
		return tls_mbedtls_clienthello_session_ticket_prep(conn, data,
		                                                   data_len);

	return -1;
}

#endif /* TLS_MBEDTLS_SESSION_TICKETS */


int tls_connection_get_failed(void *tls_ctx, struct tls_connection *conn)
{
	return conn ? conn->failed : -1;
}


int tls_connection_get_read_alerts(void *tls_ctx, struct tls_connection *conn)
{
	return conn ? conn->read_alerts : -1;
}


int tls_connection_get_write_alerts(void *tls_ctx,
				    struct tls_connection *conn)
{
	return conn ? conn->write_alerts : -1;
}


#ifdef TLS_MBEDTLS_SESSION_TICKETS
int tls_connection_set_session_ticket_cb(
	void *tls_ctx, struct tls_connection *conn,
	tls_session_ticket_cb cb, void *ctx)
{
	if (!(conn->tls_conf->flags & TLS_CONN_DISABLE_SESSION_TICKET)) {
		/* (EAP-FAST and EAP-TEAP) */
		conn->session_ticket_cb = cb;
		conn->session_ticket_cb_ctx = ctx;
		return 0;
	}
	return -1;
}
#endif


int tls_get_library_version(char *buf, size_t buf_len)
{
  #ifndef MBEDTLS_VERSION_C
	const char * const ver = "n/a";
  #else
	char ver[9];
	mbedtls_version_get_string(ver);
  #endif
	return os_snprintf(buf, buf_len,
	                   "mbed TLS build=" MBEDTLS_VERSION_STRING " run=%s", ver);
}


void tls_connection_set_success_data(struct tls_connection *conn,
				     struct wpabuf *data)
{
	wpabuf_free(conn->success_data);
	conn->success_data = data;
}


void tls_connection_set_success_data_resumed(struct tls_connection *conn)
{
}


const struct wpabuf *
tls_connection_get_success_data(struct tls_connection *conn)
{
	return conn->success_data;
}


void tls_connection_remove_session(struct tls_connection *conn)
{
}


#ifdef TLS_MBEDTLS_EAP_TEAP
int tls_get_tls_unique(struct tls_connection *conn, u8 *buf, size_t max_len)
{
  #if defined(MBEDTLS_SSL_RENEGOTIATION) /* XXX: renegotiation or resumption? */
	/* data from TLS handshake Finished message */
	size_t verify_len = conn->ssl.MBEDTLS_PRIVATE(verify_data_len);
	char *verify_data = (conn->is_server ^ conn->resumed)
	  ? conn->ssl.MBEDTLS_PRIVATE(peer_verify_data)
	  : conn->ssl.MBEDTLS_PRIVATE(own_verify_data);
	if (verify_len && verify_len <= max_len) {
		os_memcpy(buf, verify_data, verify_len);
		return (int)verify_len;
	}
  #endif
	return -1;
}
#endif


__attribute_noinline__
static void tls_mbedtls_set_peer_subject(struct tls_connection *conn, const mbedtls_x509_crt *crt)
{
	if (conn->peer_subject)
		return;
	char buf[MBEDTLS_X509_MAX_DN_NAME_SIZE*2];
	int buflen = mbedtls_x509_dn_gets(buf, sizeof(buf), &crt->subject);
	if (buflen >= 0 && (conn->peer_subject = os_malloc((size_t)buflen+1)))
		os_memcpy(conn->peer_subject, buf, (size_t)buflen+1);
}


#ifdef TLS_MBEDTLS_EAP_TEAP
const char * tls_connection_get_peer_subject(struct tls_connection *conn)
{
	if (!conn)
		return NULL;
	if (!conn->peer_subject) { /*(if not set during cert verify)*/
		const mbedtls_x509_crt *peer_cert =
		  mbedtls_ssl_get_peer_cert(&conn->ssl);
		if (peer_cert)
			tls_mbedtls_set_peer_subject(conn, peer_cert);
	}
	return conn->peer_subject;
}
#endif


#ifdef TLS_MBEDTLS_EAP_TEAP
bool tls_connection_get_own_cert_used(struct tls_connection *conn)
{
	/* XXX: availability of cert does not necessary mean that client
	 * received certificate request from server and then sent cert.
	 * ? step handshake in tls_connection_handshake() looking for
	 *   MBEDTLS_SSL_CERTIFICATE_REQUEST ? */
	const struct tls_conf * const tls_conf = conn->tls_conf;
	return (tls_conf->has_client_cert && tls_conf->has_private_key);
}
#endif


#if defined(CONFIG_FIPS)
#define TLS_MBEDTLS_CONFIG_FIPS
#endif

#if defined(CONFIG_SHA256)
#define TLS_MBEDTLS_TLS_PRF_SHA256
#endif

#if defined(CONFIG_SHA384)
#define TLS_MBEDTLS_TLS_PRF_SHA384
#endif


#ifndef TLS_MBEDTLS_CONFIG_FIPS
#if defined(CONFIG_MODULE_TESTS)
/* unused with CONFIG_TLS=mbedtls except in crypto_module_tests.c */
#if MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */ \
 && MBEDTLS_VERSION_NUMBER <  0x03000000 /* mbedtls 3.0.0 */
/* sha1-tlsprf.c */
#include "sha1.h"
int tls_prf_sha1_md5(const u8 *secret, size_t secret_len, const char *label,
		     const u8 *seed, size_t seed_len, u8 *out, size_t outlen)
{
	return mbedtls_ssl_tls_prf(MBEDTLS_SSL_TLS_PRF_TLS1,
				   secret, secret_len, label,
				   seed, seed_len, out, outlen) ? -1 : 0;
}
#else
#include "sha1-tlsprf.c" /* pull in hostap local implementation */
#endif
#endif
#endif

#ifdef TLS_MBEDTLS_TLS_PRF_SHA256
/* sha256-tlsprf.c */
#if MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */
#include "sha256.h"
int tls_prf_sha256(const u8 *secret, size_t secret_len, const char *label,
		   const u8 *seed, size_t seed_len, u8 *out, size_t outlen)
{
	return mbedtls_ssl_tls_prf(MBEDTLS_SSL_TLS_PRF_SHA256,
				   secret, secret_len, label,
				   seed, seed_len, out, outlen) ? -1 : 0;
}
#else
#include "sha256-tlsprf.c" /* pull in hostap local implementation */
#endif
#endif

#ifdef TLS_MBEDTLS_TLS_PRF_SHA384
/* sha384-tlsprf.c */
#if MBEDTLS_VERSION_NUMBER >= 0x02120000 /* mbedtls 2.18.0 */
#include "sha384.h"
int tls_prf_sha384(const u8 *secret, size_t secret_len, const char *label,
		   const u8 *seed, size_t seed_len, u8 *out, size_t outlen)
{
	return mbedtls_ssl_tls_prf(MBEDTLS_SSL_TLS_PRF_SHA384,
				   secret, secret_len, label,
				   seed, seed_len, out, outlen) ? -1 : 0;
}
#else
#include "sha384-tlsprf.c" /* pull in hostap local implementation */
#endif
#endif


#if MBEDTLS_VERSION_NUMBER < 0x03020000 /* mbedtls 3.2.0 */
#define mbedtls_x509_crt_has_ext_type(crt, ext_type) \
        ((crt)->MBEDTLS_PRIVATE(ext_types) & (ext_type))
#endif

struct mlist { const char *p; size_t n; };


static int
tls_mbedtls_match_altsubject(mbedtls_x509_crt *crt, const char *match)
{
	/* RFE: this could be pre-parsed into structured data at config time */
	struct mlist list[256]; /*(much larger than expected)*/
	int nlist = 0;
	if (   os_strncmp(match, "EMAIL:", 6) != 0
	    && os_strncmp(match, "DNS:",   4) != 0
	    && os_strncmp(match, "URI:",   4) != 0   ) {
		wpa_printf(MSG_INFO, "MTLS: Invalid altSubjectName match '%s'", match);
		return 0;
	}
	for (const char *s = match, *tok; *s; s = tok ? tok+1 : "") {
		do { } while ((tok = os_strchr(s, ';'))
		              && os_strncmp(tok+1, "EMAIL:", 6) != 0
		              && os_strncmp(tok+1, "DNS:",   4) != 0
		              && os_strncmp(tok+1, "URI:",   4) != 0);
		list[nlist].p = s;
		list[nlist].n = tok ? (size_t)(tok - s) : os_strlen(s);
		if (list[nlist].n && ++nlist == sizeof(list)/sizeof(*list)) {
			wpa_printf(MSG_INFO, "MTLS: excessive altSubjectName match '%s'",
			           match);
			break; /* truncate huge list and continue */
		}
	}

	if (!mbedtls_x509_crt_has_ext_type(crt, MBEDTLS_X509_EXT_SUBJECT_ALT_NAME))
		return 0;

	const mbedtls_x509_sequence *cur = &crt->subject_alt_names;
	for (; cur != NULL; cur = cur->next) {
		const unsigned char san_type = (unsigned char)cur->buf.tag
		                             & MBEDTLS_ASN1_TAG_VALUE_MASK;
		char t;
		size_t step = 4;
		switch (san_type) {             /* "EMAIL:" or "DNS:" or "URI:" */
		case MBEDTLS_X509_SAN_RFC822_NAME:       step = 6; t = 'E'; break;
		case MBEDTLS_X509_SAN_DNS_NAME:                    t = 'D'; break;
		case MBEDTLS_X509_SAN_UNIFORM_RESOURCE_IDENTIFIER: t = 'U'; break;
		default: continue;
		}

		for (int i = 0; i < nlist; ++i) {
			/* step over "EMAIL:" or "DNS:" or "URI:" in list[i].p */
			/* Note: v is not '\0'-terminated, but is a known length vlen,
			 * so okay to pass to os_strncasecmp() even though not z-string */
			if (cur->buf.len == list[i].n - step && t == *list[i].p
			    && 0 == os_strncasecmp((char *)cur->buf.p,
			                           list[i].p+step, cur->buf.len)) {
				return 1; /* match */
			}
		}
	}
	return 0; /* no match */
}


static int
tls_mbedtls_match_suffix(const char *v, size_t vlen,
                         const struct mlist *list, int nlist, int full)
{
	/* Note: v is not '\0'-terminated, but is a known length vlen,
	 * so okay to pass to os_strncasecmp() even though not z-string */
	for (int i = 0; i < nlist; ++i) {
		size_t n = list[i].n;
		if ((n == vlen || (n < vlen && v[vlen-n-1] == '.' && !full))
		    && 0 == os_strncasecmp(v+vlen-n, list[i].p, n))
			return 1; /* match */
	}
	return 0; /* no match */
}


static int
tls_mbedtls_match_suffixes(mbedtls_x509_crt *crt, const char *match, int full)
{
	/* RFE: this could be pre-parsed into structured data at config time */
	struct mlist list[256]; /*(much larger than expected)*/
	int nlist = 0;
	for (const char *s = match, *tok; *s; s = tok ? tok+1 : "") {
		tok = os_strchr(s, ';');
		list[nlist].p = s;
		list[nlist].n = tok ? (size_t)(tok - s) : os_strlen(s);
		if (list[nlist].n && ++nlist == sizeof(list)/sizeof(*list)) {
			wpa_printf(MSG_INFO, "MTLS: excessive suffix match '%s'", match);
			break; /* truncate huge list and continue */
		}
	}

	/* check subjectAltNames */
	if (mbedtls_x509_crt_has_ext_type(crt, MBEDTLS_X509_EXT_SUBJECT_ALT_NAME)) {
		const mbedtls_x509_sequence *cur = &crt->subject_alt_names;
		for (; cur != NULL; cur = cur->next) {
			const unsigned char san_type = (unsigned char)cur->buf.tag
			                             & MBEDTLS_ASN1_TAG_VALUE_MASK;
			if (san_type == MBEDTLS_X509_SAN_DNS_NAME
			    && tls_mbedtls_match_suffix((char *)cur->buf.p,
			                                cur->buf.len,
			                                list, nlist, full)) {
				return 1; /* match */
			}
		}
	}

	/* check subject CN */
	const mbedtls_x509_name *name = &crt->subject;
	for (; name != NULL; name = name->next) {
		if (name->oid.p && MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &name->oid) == 0)
			break;
	}
	if (name && tls_mbedtls_match_suffix((char *)name->val.p, name->val.len,
	                                     list, nlist, full)) {
		return 1; /* match */
	}

	return 0; /* no match */
}


static int
tls_mbedtls_match_dn_field(mbedtls_x509_crt *crt, const char *match)
{
	/* RFE: this could be pre-parsed into structured data at config time */
	struct mlistoid { const char *p; size_t n;
	                  const char *oid; size_t olen;
	                  int prefix; };
	struct mlistoid list[32]; /*(much larger than expected)*/
	int nlist = 0;
	for (const char *s = match, *tok, *e; *s; s = tok ? tok+1 : "") {
		tok = os_strchr(s, '/');
		list[nlist].oid = NULL;
		list[nlist].olen = 0;
		list[nlist].n = tok ? (size_t)(tok - s) : os_strlen(s);
		e = memchr(s, '=', list[nlist].n);
		if (e == NULL) {
			if (list[nlist].n == 0)
				continue; /* skip consecutive, repeated '/' */
			if (list[nlist].n == 1 && *s == '*') {
				/* special-case "*" to match any OID and value */
				s = e = "=*";
				list[nlist].n = 2;
				list[nlist].oid = "";
			}
			else {
				wpa_printf(MSG_INFO,
				           "MTLS: invalid check_cert_subject '%s' missing '='",
				           match);
				return 0;
			}
		}
		switch (e - s) {
		case 1:
			if (*s == 'C') {
				list[nlist].oid  = MBEDTLS_OID_AT_COUNTRY;
				list[nlist].olen = sizeof(MBEDTLS_OID_AT_COUNTRY)-1;
			}
			else if (*s == 'L') {
				list[nlist].oid  = MBEDTLS_OID_AT_LOCALITY;
				list[nlist].olen = sizeof(MBEDTLS_OID_AT_LOCALITY)-1;
			}
			else if (*s == 'O') {
				list[nlist].oid  = MBEDTLS_OID_AT_ORGANIZATION;
				list[nlist].olen = sizeof(MBEDTLS_OID_AT_ORGANIZATION)-1;
			}
			break;
		case 2:
			if (s[0] == 'C' && s[1] == 'N') {
				list[nlist].oid  = MBEDTLS_OID_AT_CN;
				list[nlist].olen = sizeof(MBEDTLS_OID_AT_CN)-1;
			}
			else if (s[0] == 'S' && s[1] == 'T') {
				list[nlist].oid  = MBEDTLS_OID_AT_STATE;
				list[nlist].olen = sizeof(MBEDTLS_OID_AT_STATE)-1;
			}
			else if (s[0] == 'O' && s[1] == 'U') {
				list[nlist].oid  = MBEDTLS_OID_AT_ORG_UNIT;
				list[nlist].olen = sizeof(MBEDTLS_OID_AT_ORG_UNIT)-1;
			}
			break;
		case 12:
			if (os_memcmp(s, "emailAddress", 12) == 0) {
				list[nlist].oid  = MBEDTLS_OID_PKCS9_EMAIL;
				list[nlist].olen = sizeof(MBEDTLS_OID_PKCS9_EMAIL)-1;
			}
			break;
		default:
			break;
		}
		if (list[nlist].oid == NULL) {
			wpa_printf(MSG_INFO,
			           "MTLS: Unknown field in check_cert_subject '%s'",
			           match);
			return 0;
		}
		list[nlist].n -= (size_t)(++e - s);
		list[nlist].p = e;
		if (list[nlist].n && e[list[nlist].n-1] == '*') {
			--list[nlist].n;
			list[nlist].prefix = 1;
		}
		/*(could easily add support for suffix matches if value begins with '*',
		 * but suffix match is not currently supported by other TLS modules)*/

		if (list[nlist].n && ++nlist == sizeof(list)/sizeof(*list)) {
			wpa_printf(MSG_INFO,
			           "MTLS: excessive check_cert_subject match '%s'",
			           match);
			break; /* truncate huge list and continue */
		}
	}

	/* each component in match string must match cert Subject in order listed
	 * The behavior below preserves ordering but is slightly different than
	 * the grossly inefficient contortions implemented in tls_openssl.c */
	const mbedtls_x509_name *name = &crt->subject;
	for (int i = 0; i < nlist; ++i) {
		int found = 0;
		for (; name != NULL && !found; name = name->next) {
			if (!name->oid.p)
				continue;
			/* special-case "*" to match any OID and value */
			if (list[i].olen == 0) {
				found = 1;
				continue;
			}
			/* perform equalent of !MBEDTLS_OID_CMP() with oid ptr and len */
			if (list[i].olen != name->oid.len
			    || os_memcmp(list[i].oid, name->oid.p, name->oid.len) != 0)
				continue;
			/* Note: v is not '\0'-terminated, but is a known length vlen,
			 * so okay to pass to os_strncasecmp() even though not z-string */
			if ((list[i].prefix
			      ? list[i].n <= name->val.len  /* prefix match */
			      : list[i].n == name->val.len) /* full match */
			    && 0 == os_strncasecmp((char *)name->val.p,
			                           list[i].p, list[i].n)) {
				found = 1;
				continue;
			}
		}
		if (!found)
			return 0; /* no match */
	}
	return 1; /* match */
}


__attribute_cold__
static void
tls_mbedtls_verify_fail_event (mbedtls_x509_crt *crt, int depth,
                               const char *errmsg, enum tls_fail_reason reason)
{
	struct tls_config *init_conf = &tls_ctx_global.init_conf;
	if (init_conf->event_cb == NULL)
		return;

	struct wpabuf *certbuf = wpabuf_alloc_copy(crt->raw.p, crt->raw.len);
	char subject[MBEDTLS_X509_MAX_DN_NAME_SIZE*2];
	if (mbedtls_x509_dn_gets(subject, sizeof(subject), &crt->subject) < 0)
		subject[0] = '\0';
	union tls_event_data ev;
	os_memset(&ev, 0, sizeof(ev));
	ev.cert_fail.reason = reason;
	ev.cert_fail.depth = depth;
	ev.cert_fail.subject = subject;
	ev.cert_fail.reason_txt = errmsg;
	ev.cert_fail.cert = certbuf;

	init_conf->event_cb(init_conf->cb_ctx, TLS_CERT_CHAIN_FAILURE, &ev);

	wpabuf_free(certbuf);
}


__attribute_noinline__
static void
tls_mbedtls_verify_cert_event (struct tls_connection *conn,
                               mbedtls_x509_crt *crt, int depth)
{
	struct tls_config *init_conf = &tls_ctx_global.init_conf;
	if (init_conf->event_cb == NULL)
		return;

	struct wpabuf *certbuf = NULL;
	union tls_event_data ev;
	os_memset(&ev, 0, sizeof(ev));

  #ifdef MBEDTLS_SHA256_C
	u8 hash[SHA256_DIGEST_LENGTH];
	const u8 *addr[] = { (u8 *)crt->raw.p };
	if (sha256_vector(1, addr, &crt->raw.len, hash) == 0) {
		ev.peer_cert.hash = hash;
		ev.peer_cert.hash_len = sizeof(hash);
	}
  #endif
	ev.peer_cert.depth = depth;
	char subject[MBEDTLS_X509_MAX_DN_NAME_SIZE*2];
	if (depth == 0)
		ev.peer_cert.subject = conn->peer_subject;
	if (ev.peer_cert.subject == NULL) {
		ev.peer_cert.subject = subject;
		if (mbedtls_x509_dn_gets(subject, sizeof(subject), &crt->subject) < 0)
			subject[0] = '\0';
	}

	char serial_num[128+1];
	ev.peer_cert.serial_num =
	  tls_mbedtls_peer_serial_num(crt, serial_num, sizeof(serial_num));

	const mbedtls_x509_sequence *cur;

	cur = NULL;
	if (mbedtls_x509_crt_has_ext_type(crt, MBEDTLS_X509_EXT_SUBJECT_ALT_NAME))
		cur = &crt->subject_alt_names;
	for (; cur != NULL; cur = cur->next) {
		const unsigned char san_type = (unsigned char)cur->buf.tag
		                             & MBEDTLS_ASN1_TAG_VALUE_MASK;
		size_t prelen = 4;
		const char *pre;
		switch (san_type) {
		case MBEDTLS_X509_SAN_RFC822_NAME:     prelen = 6; pre = "EMAIL:";break;
		case MBEDTLS_X509_SAN_DNS_NAME:                    pre = "DNS:";  break;
		case MBEDTLS_X509_SAN_UNIFORM_RESOURCE_IDENTIFIER: pre = "URI:";  break;
		default: continue;
		}

		char *pos = os_malloc(prelen + cur->buf.len + 1);
		if (pos == NULL)
			break;
		ev.peer_cert.altsubject[ev.peer_cert.num_altsubject] = pos;
		os_memcpy(pos, pre, prelen);
		/* data should be properly backslash-escaped if needed,
		 * so code below does not re-escape, but does replace CTLs */
		/*os_memcpy(pos+prelen, cur->buf.p, cur->buf.len);*/
		/*pos[prelen+cur->buf.len] = '\0';*/
		pos += prelen;
		for (size_t i = 0; i < cur->buf.len; ++i) {
			unsigned char c = cur->buf.p[i];
			*pos++ = (c >= 32 && c != 127) ? c : '?';
		}
		*pos = '\0';

		if (++ev.peer_cert.num_altsubject == TLS_MAX_ALT_SUBJECT)
			break;
	}

	cur = NULL;
	if (mbedtls_x509_crt_has_ext_type(crt, MBEDTLS_X509_EXT_CERTIFICATE_POLICIES))
		cur = &crt->certificate_policies;
	for (; cur != NULL; cur = cur->next) {
		if (cur->buf.len != 11) /* len of OID_TOD_STRICT or OID_TOD_TOFU */
			continue;
		/* TOD-STRICT "1.3.6.1.4.1.40808.1.3.1" */
		/* TOD-TOFU   "1.3.6.1.4.1.40808.1.3.2" */
		#define OID_TOD_STRICT "\x2b\x06\x01\x04\x01\x82\xbe\x68\x01\x03\x01"
		#define OID_TOD_TOFU   "\x2b\x06\x01\x04\x01\x82\xbe\x68\x01\x03\x02"
		if (os_memcmp(cur->buf.p,
		              OID_TOD_STRICT, sizeof(OID_TOD_STRICT)-1) == 0) {
			ev.peer_cert.tod = 1; /* TOD-STRICT */
			break;
		}
		if (os_memcmp(cur->buf.p,
		              OID_TOD_TOFU, sizeof(OID_TOD_TOFU)-1) == 0) {
			ev.peer_cert.tod = 2; /* TOD-TOFU */
			break;
		}
	}

	struct tls_conf *tls_conf = conn->tls_conf;
	if (tls_conf->ca_cert_probe || (tls_conf->flags & TLS_CONN_EXT_CERT_CHECK)
	    || init_conf->cert_in_cb) {
		certbuf = wpabuf_alloc_copy(crt->raw.p, crt->raw.len);
		ev.peer_cert.cert = certbuf;
	}

	init_conf->event_cb(init_conf->cb_ctx, TLS_PEER_CERTIFICATE, &ev);

	wpabuf_free(certbuf);
	char **altsubject;
	*(const char ***)&altsubject = ev.peer_cert.altsubject;
	for (size_t i = 0; i < ev.peer_cert.num_altsubject; ++i)
		os_free(altsubject[i]);
}


static int
tls_mbedtls_verify_cb (void *arg, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
	/* XXX: N.B. verify code not carefully tested besides hwsim tests
	 *
	 * RFE: mbedtls_x509_crt_verify_info() and enhance log trace messages
	 * RFE: review and add support for additional TLS_CONN_* flags
	 * not handling OCSP (not available in mbedtls)
	 * ... */

	struct tls_connection *conn = (struct tls_connection *)arg;
	struct tls_conf *tls_conf = conn->tls_conf;
	uint32_t flags_in = *flags;

	if (depth > 8) { /*(depth 8 picked as arbitrary limit)*/
		emsg(MSG_WARNING, "client cert chain too long");
		*flags |= MBEDTLS_X509_BADCERT_OTHER; /* cert chain too long */
		tls_mbedtls_verify_fail_event(crt, depth,
			                      "client cert chain too long",
		                              TLS_FAIL_BAD_CERTIFICATE);
	}
	else if (tls_conf->verify_depth0_only) {
		if (depth > 0)
			*flags = 0;
		else {
		  #ifdef MBEDTLS_SHA256_C
			u8 hash[SHA256_DIGEST_LENGTH];
			const u8 *addr[] = { (u8 *)crt->raw.p };
			if (sha256_vector(1, addr, &crt->raw.len, hash) < 0
			    || os_memcmp(tls_conf->ca_cert_hash, hash, sizeof(hash)) != 0) {
				*flags |= MBEDTLS_X509_BADCERT_NOT_TRUSTED;
				tls_mbedtls_verify_fail_event(crt, depth,
			                                      "cert hash mismatch",
				                              TLS_FAIL_UNTRUSTED);
			}
			else /* hash matches; ignore other issues *except* if revoked)*/
				*flags &= MBEDTLS_X509_BADCERT_REVOKED;
		  #endif
		}
	}
	else if (depth == 0) {
		if (!conn->peer_subject)
			tls_mbedtls_set_peer_subject(conn, crt);
		/*(use same labels to tls_mbedtls_verify_fail_event() as used in
		 * other TLS modules so that hwsim tests find exact string match)*/
		if (!conn->peer_subject) { /* error copying subject string */
			*flags |= MBEDTLS_X509_BADCERT_OTHER;
			tls_mbedtls_verify_fail_event(crt, depth,
			                              "internal error",
			                              TLS_FAIL_UNSPECIFIED);
		}
		/*(use os_strstr() for subject match as is done in tls_mbedtls.c
		 * to follow the same behavior, even though a suffix match would
		 * make more sense.  Also, note that strstr match does not
		 * normalize whitespace (between components) for comparison)*/
		else if (tls_conf->subject_match
		         && os_strstr(conn->peer_subject,
		                      tls_conf->subject_match) == NULL) {
			wpa_printf(MSG_WARNING,
			           "MTLS: Subject '%s' did not match with '%s'",
			           conn->peer_subject, tls_conf->subject_match);
			*flags |= MBEDTLS_X509_BADCERT_CN_MISMATCH;
			tls_mbedtls_verify_fail_event(crt, depth,
			                              "Subject mismatch",
			                              TLS_FAIL_SUBJECT_MISMATCH);
		}
		if (tls_conf->altsubject_match
		    && !tls_mbedtls_match_altsubject(crt, tls_conf->altsubject_match)) {
			wpa_printf(MSG_WARNING,
				   "MTLS: altSubjectName match '%s' not found",
			           tls_conf->altsubject_match);
			*flags |= MBEDTLS_X509_BADCERT_CN_MISMATCH;
			tls_mbedtls_verify_fail_event(crt, depth,
			                              "AltSubject mismatch",
			                              TLS_FAIL_ALTSUBJECT_MISMATCH);
		}
		if (tls_conf->suffix_match
		    && !tls_mbedtls_match_suffixes(crt, tls_conf->suffix_match, 0)) {
			wpa_printf(MSG_WARNING,
			           "MTLS: Domain suffix match '%s' not found",
				   tls_conf->suffix_match);
			*flags |= MBEDTLS_X509_BADCERT_CN_MISMATCH;
			tls_mbedtls_verify_fail_event(crt, depth,
			                              "Domain suffix mismatch",
			                              TLS_FAIL_DOMAIN_SUFFIX_MISMATCH);
		}
		if (tls_conf->domain_match
		    && !tls_mbedtls_match_suffixes(crt, tls_conf->domain_match, 1)) {
			wpa_printf(MSG_WARNING,
			           "MTLS: Domain match '%s' not found",
				   tls_conf->domain_match);
			*flags |= MBEDTLS_X509_BADCERT_CN_MISMATCH;
			tls_mbedtls_verify_fail_event(crt, depth,
			                              "Domain mismatch",
			                              TLS_FAIL_DOMAIN_MISMATCH);
		}
		if (tls_conf->check_cert_subject
		    && !tls_mbedtls_match_dn_field(crt, tls_conf->check_cert_subject)) {
			*flags |= MBEDTLS_X509_BADCERT_CN_MISMATCH;
			tls_mbedtls_verify_fail_event(crt, depth,
			                              "Distinguished Name",
			                              TLS_FAIL_DN_MISMATCH);
		}
		if (tls_conf->flags & TLS_CONN_SUITEB) {
			/* check RSA modulus size (public key bitlen) */
			const mbedtls_pk_type_t pk_alg = mbedtls_pk_get_type(&crt->pk);
			if ((pk_alg == MBEDTLS_PK_RSA || pk_alg == MBEDTLS_PK_RSASSA_PSS)
			    && mbedtls_pk_get_bitlen(&crt->pk) < 3072) {
				/* hwsim suite_b RSA tests expect 3072
				 *   suite_b_192_rsa_ecdhe_radius_rsa2048_client
				 *   suite_b_192_rsa_dhe_radius_rsa2048_client */
				*flags |= MBEDTLS_X509_BADCERT_BAD_KEY;
				tls_mbedtls_verify_fail_event(crt, depth,
				                              "Insufficient RSA modulus size",
				                              TLS_FAIL_INSUFFICIENT_KEY_LEN);
			}
		}
		if (tls_conf->check_crl && tls_conf->crl == NULL) {
			/* see tests/hwsim test_ap_eap.py ap_wpa2_eap_tls_check_crl */
			emsg(MSG_WARNING, "check_crl set but no CRL loaded; reject all?");
			*flags |= MBEDTLS_X509_BADCERT_OTHER;
			tls_mbedtls_verify_fail_event(crt, depth,
				                      "check_crl set but no CRL loaded; "
			                              "reject all?",
			                              TLS_FAIL_BAD_CERTIFICATE);
		}
	}
	else {
		if (tls_conf->check_crl != 2) /* 2 == verify CRLs for all certs */
			*flags &= ~MBEDTLS_X509_BADCERT_REVOKED;
	}

	if (!tls_conf->check_crl_strict) {
		*flags &= ~MBEDTLS_X509_BADCRL_EXPIRED;
		*flags &= ~MBEDTLS_X509_BADCRL_FUTURE;
	}

	if (tls_conf->flags & TLS_CONN_DISABLE_TIME_CHECKS) {
		*flags &= ~MBEDTLS_X509_BADCERT_EXPIRED;
		*flags &= ~MBEDTLS_X509_BADCERT_FUTURE;
	}

	tls_mbedtls_verify_cert_event(conn, crt, depth);

	if (*flags) {
		if (*flags & (MBEDTLS_X509_BADCERT_NOT_TRUSTED
		             |MBEDTLS_X509_BADCERT_CN_MISMATCH
		             |MBEDTLS_X509_BADCERT_REVOKED)) {
			emsg(MSG_WARNING, "client cert not trusted");
		}
		/* report event if flags set but no additional flags set above */
		/* (could translate flags to more detailed TLS_FAIL_* if needed) */
		if (!(*flags & ~flags_in)) {
			enum tls_fail_reason reason = TLS_FAIL_UNSPECIFIED;
			const char *errmsg = "cert verify fail unspecified";
			if (*flags & MBEDTLS_X509_BADCERT_NOT_TRUSTED) {
				reason = TLS_FAIL_UNTRUSTED;
				errmsg = "certificate not trusted";
			}
			if (*flags & MBEDTLS_X509_BADCERT_REVOKED) {
				reason = TLS_FAIL_REVOKED;
				errmsg = "certificate has been revoked";
			}
			if (*flags & MBEDTLS_X509_BADCERT_FUTURE) {
				reason = TLS_FAIL_NOT_YET_VALID;
				errmsg = "certificate not yet valid";
			}
			if (*flags & MBEDTLS_X509_BADCERT_EXPIRED) {
				reason = TLS_FAIL_EXPIRED;
				errmsg = "certificate has expired";
			}
			if (*flags & MBEDTLS_X509_BADCERT_BAD_MD) {
				reason = TLS_FAIL_BAD_CERTIFICATE;
				errmsg = "certificate uses insecure algorithm";
			}
			tls_mbedtls_verify_fail_event(crt, depth, errmsg, reason);
		}
	  #if 0
		/* ??? send (again) cert events for all certs in chain ???
		 * (should already have been called for greater depths) */
		/* tls_openssl.c:tls_verify_cb() sends cert events for all certs
		 * in chain if certificate validation fails, but sends all events
		 * with depth set to 0 (might be a bug) */
		if (depth > 0) {
			int pdepth = depth + 1;
			for (mbedtls_x509_crt *pcrt; (pcrt = crt->next); ++pdepth) {
				tls_mbedtls_verify_cert_event(conn, pcrt, pdepth);
			}
		}
	  #endif
		/*(do not preserve subject if verification failed but was optional)*/
		if (depth == 0 && conn->peer_subject) {
			os_free(conn->peer_subject);
			conn->peer_subject = NULL;
		}
	}
	else if (depth == 0) {
		struct tls_config *init_conf = &tls_ctx_global.init_conf;
		if (tls_conf->ca_cert_probe) {
			/* reject server certificate on probe-only run */
			*flags |= MBEDTLS_X509_BADCERT_OTHER;
			tls_mbedtls_verify_fail_event(crt, depth,
			                              "server chain probe",
			                              TLS_FAIL_SERVER_CHAIN_PROBE);
		}
		else if (init_conf->event_cb) {
			/* ??? send event as soon as depth == 0 is verified ???
			 * What about rest of chain?
			 * Follows tls_mbedtls.c behavior: */
			init_conf->event_cb(init_conf->cb_ctx,
			                    TLS_CERT_CHAIN_SUCCESS, NULL);
		}
	}

	return 0;
}
