/*
 * wpa_supplicant: TLSv1 client (RFC 2246)
 * Copyright (c) 2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"
#include "md5.h"
#include "sha1.h"
#include "crypto.h"
#include "tls.h"
#include "tlsv1_common.h"
#include "tlsv1_client.h"

/* TODO:
 * Server certificate chain validation
 * Client certificate configuration
 * CertificateVerify
 * Support for a message fragmented across several records (RFC 2246, 6.2.1)
 */

struct tlsv1_client {
	enum {
		CLIENT_HELLO, SERVER_HELLO, SERVER_CERTIFICATE,
		SERVER_KEY_EXCHANGE, SERVER_CERTIFICATE_REQUEST,
		SERVER_HELLO_DONE, CLIENT_KEY_EXCHANGE, CHANGE_CIPHER_SPEC,
		SERVER_CHANGE_CIPHER_SPEC, SERVER_FINISHED, ACK_FINISHED,
		ESTABLISHED, FAILED
	} state;

	struct tlsv1_record_layer rl;

	u8 session_id[TLS_SESSION_ID_MAX_LEN];
	size_t session_id_len;
	u8 client_random[TLS_RANDOM_LEN];
	u8 server_random[TLS_RANDOM_LEN];
	u8 master_secret[TLS_MASTER_SECRET_LEN];

	u8 alert_level;
	u8 alert_description;

	unsigned int certificate_requested:1;
	unsigned int session_resumed:1;
	unsigned int ticket:1;
	unsigned int ticket_key:1;

	struct crypto_public_key *server_rsa_key;

	struct crypto_hash *verify_md5_client;
	struct crypto_hash *verify_sha1_client;
	struct crypto_hash *verify_md5_server;
	struct crypto_hash *verify_sha1_server;

#define MAX_CIPHER_COUNT 30
	u16 cipher_suites[MAX_CIPHER_COUNT];
	size_t num_cipher_suites;

	u16 prev_cipher_suite;

	u8 *client_hello_ext;
	size_t client_hello_ext_len;

	/* The prime modulus used for Diffie-Hellman */
	u8 *dh_p;
	size_t dh_p_len;
	/* The generator used for Diffie-Hellman */
	u8 *dh_g;
	size_t dh_g_len;
	/* The server's Diffie-Hellman public value */
	u8 *dh_ys;
	size_t dh_ys_len;
};


static int tls_derive_keys(struct tlsv1_client *conn,
			   const u8 *pre_master_secret,
			   size_t pre_master_secret_len);
static int tls_process_server_key_exchange(struct tlsv1_client *conn, u8 ct,
					   const u8 *in_data, size_t *in_len);
static int tls_process_certificate_request(struct tlsv1_client *conn, u8 ct,
					   const u8 *in_data, size_t *in_len);
static int tls_process_server_hello_done(struct tlsv1_client *conn, u8 ct,
					 const u8 *in_data, size_t *in_len);


static void tls_alert(struct tlsv1_client *conn, u8 level, u8 description)
{
	conn->alert_level = level;
	conn->alert_description = description;
}


static void tls_verify_hash_add(struct tlsv1_client *conn, const u8 *buf,
				size_t len)
{
	if (conn->verify_md5_client && conn->verify_sha1_client) {
		crypto_hash_update(conn->verify_md5_client, buf, len);
		crypto_hash_update(conn->verify_sha1_client, buf, len);
	}
	if (conn->verify_md5_server && conn->verify_sha1_server) {
		crypto_hash_update(conn->verify_md5_server, buf, len);
		crypto_hash_update(conn->verify_sha1_server, buf, len);
	}
}


static u8 * tls_send_alert(struct tlsv1_client *conn,
			   u8 level, u8 description,
			   size_t *out_len)
{
	u8 *alert, *pos, *length;

	wpa_printf(MSG_DEBUG, "TLSv1: Send Alert(%d:%d)", level, description);
	*out_len = 0;

	alert = malloc(10);
	if (alert == NULL)
		return NULL;

	pos = alert;

	/* TLSPlaintext */
	/* ContentType type */
	*pos++ = TLS_CONTENT_TYPE_ALERT;
	/* ProtocolVersion version */
	WPA_PUT_BE16(pos, TLS_VERSION);
	pos += 2;
	/* uint16 length (to be filled) */
	length = pos;
	pos += 2;
	/* opaque fragment[TLSPlaintext.length] */

	/* Alert */
	/* AlertLevel level */
	*pos++ = level;
	/* AlertDescription description */
	*pos++ = description;

	WPA_PUT_BE16(length, pos - length - 2);
	*out_len = pos - alert;

	return alert;
}


static u8 * tls_send_client_hello(struct tlsv1_client *conn,
				  size_t *out_len)
{
	u8 *hello, *end, *pos, *hs_length, *hs_start, *rhdr;
	struct os_time now;
	size_t len, i;

	wpa_printf(MSG_DEBUG, "TLSv1: Send ClientHello");
	*out_len = 0;

	os_get_time(&now);
	WPA_PUT_BE32(conn->client_random, now.sec);
	if (os_get_random(conn->client_random + 4, TLS_RANDOM_LEN - 4)) {
		wpa_printf(MSG_ERROR, "TLSv1: Could not generate "
			   "client_random");
		return NULL;
	}
	wpa_hexdump(MSG_MSGDUMP, "TLSv1: client_random",
		    conn->client_random, TLS_RANDOM_LEN);

	len = 100 + conn->num_cipher_suites * 2 + conn->client_hello_ext_len;
	hello = malloc(len);
	if (hello == NULL)
		return NULL;
	end = hello + len;

	rhdr = hello;
	pos = rhdr + TLS_RECORD_HEADER_LEN;

	/* opaque fragment[TLSPlaintext.length] */

	/* Handshake */
	hs_start = pos;
	/* HandshakeType msg_type */
	*pos++ = TLS_HANDSHAKE_TYPE_CLIENT_HELLO;
	/* uint24 length (to be filled) */
	hs_length = pos;
	pos += 3;
	/* body - ClientHello */
	/* ProtocolVersion client_version */
	WPA_PUT_BE16(pos, TLS_VERSION);
	pos += 2;
	/* Random random: uint32 gmt_unix_time, opaque random_bytes */
	memcpy(pos, conn->client_random, TLS_RANDOM_LEN);
	pos += TLS_RANDOM_LEN;
	/* SessionID session_id */
	*pos++ = conn->session_id_len;
	memcpy(pos, conn->session_id, conn->session_id_len);
	pos += conn->session_id_len;
	/* CipherSuite cipher_suites<2..2^16-1> */
	WPA_PUT_BE16(pos, 2 * conn->num_cipher_suites);
	pos += 2;
	for (i = 0; i < conn->num_cipher_suites; i++) {
		WPA_PUT_BE16(pos, conn->cipher_suites[i]);
		pos += 2;
	}
	/* CompressionMethod compression_methods<1..2^8-1> */
	*pos++ = 1;
	*pos++ = TLS_COMPRESSION_NULL;

	if (conn->client_hello_ext) {
		memcpy(pos, conn->client_hello_ext,
		       conn->client_hello_ext_len);
		pos += conn->client_hello_ext_len;
	}

	WPA_PUT_BE24(hs_length, pos - hs_length - 3);
	tls_verify_hash_add(conn, hs_start, pos - hs_start);

	if (tlsv1_record_send(&conn->rl, TLS_CONTENT_TYPE_HANDSHAKE,
			      rhdr, end - rhdr, pos - hs_start, out_len) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to create TLS record");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		free(hello);
		return NULL;
	}

	conn->state = SERVER_HELLO;

	return hello;
}


static int tls_process_server_hello(struct tlsv1_client *conn, u8 ct,
				    const u8 *in_data, size_t *in_len)
{
	const u8 *pos, *end;
	size_t left, len, i;
	u16 cipher_suite;

	if (ct != TLS_CONTENT_TYPE_HANDSHAKE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected Handshake; "
			   "received content type 0x%x", ct);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	pos = in_data;
	left = *in_len;

	if (left < 4)
		goto decode_error;

	/* HandshakeType msg_type */
	if (*pos != TLS_HANDSHAKE_TYPE_SERVER_HELLO) {
		wpa_printf(MSG_DEBUG, "TLSv1: Received unexpected handshake "
			   "message %d (expected ServerHello)", *pos);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}
	wpa_printf(MSG_DEBUG, "TLSv1: Received ServerHello");
	pos++;
	/* uint24 length */
	len = WPA_GET_BE24(pos);
	pos += 3;
	left -= 4;

	if (len > left)
		goto decode_error;

	/* body - ServerHello */

	wpa_hexdump(MSG_MSGDUMP, "TLSv1: ServerHello", pos, len);
	end = pos + len;

	/* ProtocolVersion server_version */
	if (end - pos < 2)
		goto decode_error;
	if (WPA_GET_BE16(pos) != TLS_VERSION) {
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected protocol version in "
			   "ServerHello");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_PROTOCOL_VERSION);
		return -1;
	}
	pos += 2;

	/* Random random */
	if (end - pos < TLS_RANDOM_LEN)
		goto decode_error;

	memcpy(conn->server_random, pos, TLS_RANDOM_LEN);
	pos += TLS_RANDOM_LEN;
	wpa_hexdump(MSG_MSGDUMP, "TLSv1: server_random",
		    conn->server_random, TLS_RANDOM_LEN);

	/* SessionID session_id */
	if (end - pos < 1)
		goto decode_error;
	if (end - pos < 1 + *pos || *pos > TLS_SESSION_ID_MAX_LEN)
		goto decode_error;
	if (conn->session_id_len && conn->session_id_len == *pos &&
	    memcmp(conn->session_id, pos + 1, conn->session_id_len) == 0) {
		pos += 1 + conn->session_id_len;
		wpa_printf(MSG_DEBUG, "TLSv1: Resuming old session");
		conn->session_resumed = 1;
	} else {
		conn->session_id_len = *pos;
		pos++;
		memcpy(conn->session_id, pos, conn->session_id_len);
		pos += conn->session_id_len;
	}
	wpa_hexdump(MSG_MSGDUMP, "TLSv1: session_id",
		    conn->session_id, conn->session_id_len);

	/* CipherSuite cipher_suite */
	if (end - pos < 2)
		goto decode_error;
	cipher_suite = WPA_GET_BE16(pos);
	pos += 2;
	for (i = 0; i < conn->num_cipher_suites; i++) {
		if (cipher_suite == conn->cipher_suites[i])
			break;
	}
	if (i == conn->num_cipher_suites) {
		wpa_printf(MSG_INFO, "TLSv1: Server selected unexpected "
			   "cipher suite 0x%04x", cipher_suite);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_ILLEGAL_PARAMETER);
		return -1;
	}

	if (conn->session_resumed && cipher_suite != conn->prev_cipher_suite) {
		wpa_printf(MSG_DEBUG, "TLSv1: Server selected a different "
			   "cipher suite for a resumed connection (0x%04x != "
			   "0x%04x)", cipher_suite, conn->prev_cipher_suite);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_ILLEGAL_PARAMETER);
		return -1;
	}

	if (tlsv1_record_set_cipher_suite(&conn->rl, cipher_suite) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to set CipherSuite for "
			   "record layer");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	conn->prev_cipher_suite = cipher_suite;

	if (conn->session_resumed || conn->ticket_key)
		tls_derive_keys(conn, NULL, 0);

	/* CompressionMethod compression_method */
	if (end - pos < 1)
		goto decode_error;
	if (*pos != TLS_COMPRESSION_NULL) {
		wpa_printf(MSG_INFO, "TLSv1: Server selected unexpected "
			   "compression 0x%02x", *pos);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_ILLEGAL_PARAMETER);
		return -1;
	}
	pos++;

	if (end != pos) {
		wpa_hexdump(MSG_DEBUG, "TLSv1: Unexpected extra data in the "
			    "end of ServerHello", pos, end - pos);
		goto decode_error;
	}

	*in_len = end - in_data;

	conn->state = (conn->session_resumed || conn->ticket) ?
		SERVER_CHANGE_CIPHER_SPEC : SERVER_CERTIFICATE;

	return 0;

decode_error:
	wpa_printf(MSG_DEBUG, "TLSv1: Failed to decode ServerHello");
	tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
	return -1;
}


static int tls_server_key_exchange_allowed(struct tlsv1_client *conn)
{
	const struct tls_cipher_suite *suite;

	/* RFC 2246, Section 7.4.3 */
	suite = tls_get_cipher_suite(conn->rl.cipher_suite);
	if (suite == NULL)
		return 0;

	switch (suite->key_exchange) {
	case TLS_KEY_X_DHE_DSS:
	case TLS_KEY_X_DHE_DSS_EXPORT:
	case TLS_KEY_X_DHE_RSA:
	case TLS_KEY_X_DHE_RSA_EXPORT:
	case TLS_KEY_X_DH_anon_EXPORT:
	case TLS_KEY_X_DH_anon:
		return 1;
	case TLS_KEY_X_RSA_EXPORT:
		return 1 /* FIX: public key len > 512 bits */;
	default:
		return 0;
	}
}


static int tls_process_certificate(struct tlsv1_client *conn, u8 ct,
				   const u8 *in_data, size_t *in_len)
{
	const u8 *pos, *end;
	size_t left, len, list_len, cert_len, idx;
	u8 type;

	if (ct != TLS_CONTENT_TYPE_HANDSHAKE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected Handshake; "
			   "received content type 0x%x", ct);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	pos = in_data;
	left = *in_len;

	if (left < 4) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short Certificate message "
			   "(len=%lu)", (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	type = *pos++;
	len = WPA_GET_BE24(pos);
	pos += 3;
	left -= 4;

	if (len > left) {
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected Certificate message "
			   "length (len=%lu != left=%lu)",
			   (unsigned long) len, (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	if (type == TLS_HANDSHAKE_TYPE_SERVER_KEY_EXCHANGE)
		return tls_process_server_key_exchange(conn, ct, in_data,
						       in_len);
	if (type == TLS_HANDSHAKE_TYPE_CERTIFICATE_REQUEST)
		return tls_process_certificate_request(conn, ct, in_data,
						       in_len);
	if (type == TLS_HANDSHAKE_TYPE_SERVER_HELLO_DONE)
		return tls_process_server_hello_done(conn, ct, in_data,
						     in_len);
	if (type != TLS_HANDSHAKE_TYPE_CERTIFICATE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Received unexpected handshake "
			   "message %d (expected Certificate/"
			   "ServerKeyExchange/CertificateRequest/"
			   "ServerHelloDone)", type);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	wpa_printf(MSG_DEBUG,
		   "TLSv1: Received Certificate (certificate_list len %lu)",
		   (unsigned long) len);

	/*
	 * opaque ASN.1Cert<2^24-1>;
	 *
	 * struct {
	 *     ASN.1Cert certificate_list<1..2^24-1>;
	 * } Certificate;
	 */

	end = pos + len;

	if (end - pos < 3) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short Certificate "
			   "(left=%lu)", (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	list_len = WPA_GET_BE24(pos);
	pos += 3;

	if ((size_t) (end - pos) != list_len) {
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected certificate_list "
			   "length (len=%lu left=%lu)",
			   (unsigned long) list_len,
			   (unsigned long) (end - pos));
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	idx = 0;
	while (pos < end) {
		if (end - pos < 3) {
			wpa_printf(MSG_DEBUG, "TLSv1: Failed to parse "
				   "certificate_list");
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
				  TLS_ALERT_DECODE_ERROR);
			return -1;
		}

		cert_len = WPA_GET_BE24(pos);
		pos += 3;

		if ((size_t) (end - pos) < cert_len) {
			wpa_printf(MSG_DEBUG, "TLSv1: Unexpected certificate "
				   "length (len=%lu left=%lu)",
				   (unsigned long) cert_len,
				   (unsigned long) (end - pos));
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
				  TLS_ALERT_DECODE_ERROR);
			return -1;
		}

		wpa_printf(MSG_DEBUG, "TLSv1: Certificate %lu (len %lu)",
			   (unsigned long) idx, (unsigned long) cert_len);

		if (idx == 0) {
			crypto_public_key_free(conn->server_rsa_key);
			if (tls_parse_cert(pos, cert_len,
					   &conn->server_rsa_key)) {
				wpa_printf(MSG_DEBUG, "TLSv1: Failed to parse "
					   "the certificate");
				tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
					  TLS_ALERT_BAD_CERTIFICATE);
				return -1;
			}
		} else {
			/* TODO: add CAs and validate certificate change */
		}

		idx++;
		pos += cert_len;
	}

	*in_len = end - in_data;

	conn->state = SERVER_KEY_EXCHANGE;

	return 0;
}


static void tlsv1_client_free_dh(struct tlsv1_client *conn)
{
	free(conn->dh_p);
	free(conn->dh_g);
	free(conn->dh_ys);
	conn->dh_p = conn->dh_g = conn->dh_ys = NULL;
}


static int tlsv1_process_diffie_hellman(struct tlsv1_client *conn,
					const u8 *buf, size_t len)
{
	const u8 *pos, *end;

	tlsv1_client_free_dh(conn);

	pos = buf;
	end = buf + len;

	if (end - pos < 3)
		goto fail;
	conn->dh_p_len = WPA_GET_BE16(pos);
	pos += 2;
	if (conn->dh_p_len == 0 || end - pos < (int) conn->dh_p_len)
		goto fail;
	conn->dh_p = malloc(conn->dh_p_len);
	if (conn->dh_p == NULL)
		goto fail;
	memcpy(conn->dh_p, pos, conn->dh_p_len);
	pos += conn->dh_p_len;
	wpa_hexdump(MSG_DEBUG, "TLSv1: DH p (prime)",
		    conn->dh_p, conn->dh_p_len);

	if (end - pos < 3)
		goto fail;
	conn->dh_g_len = WPA_GET_BE16(pos);
	pos += 2;
	if (conn->dh_g_len == 0 || end - pos < (int) conn->dh_g_len)
		goto fail;
	conn->dh_g = malloc(conn->dh_g_len);
	if (conn->dh_g == NULL)
		goto fail;
	memcpy(conn->dh_g, pos, conn->dh_g_len);
	pos += conn->dh_g_len;
	wpa_hexdump(MSG_DEBUG, "TLSv1: DH g (generator)",
		    conn->dh_g, conn->dh_g_len);
	if (conn->dh_g_len == 1 && conn->dh_g[0] < 2)
		goto fail;

	if (end - pos < 3)
		goto fail;
	conn->dh_ys_len = WPA_GET_BE16(pos);
	pos += 2;
	if (conn->dh_ys_len == 0 || end - pos < (int) conn->dh_ys_len)
		goto fail;
	conn->dh_ys = malloc(conn->dh_ys_len);
	if (conn->dh_ys == NULL)
		goto fail;
	memcpy(conn->dh_ys, pos, conn->dh_ys_len);
	pos += conn->dh_ys_len;
	wpa_hexdump(MSG_DEBUG, "TLSv1: DH Ys (server's public value)",
		    conn->dh_ys, conn->dh_ys_len);

	return 0;

fail:
	tlsv1_client_free_dh(conn);
	return -1;
}


static int tls_process_server_key_exchange(struct tlsv1_client *conn, u8 ct,
					   const u8 *in_data, size_t *in_len)
{
	const u8 *pos, *end;
	size_t left, len;
	u8 type;
	const struct tls_cipher_suite *suite;

	if (ct != TLS_CONTENT_TYPE_HANDSHAKE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected Handshake; "
			   "received content type 0x%x", ct);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	pos = in_data;
	left = *in_len;

	if (left < 4) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short ServerKeyExchange "
			   "(Left=%lu)", (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	type = *pos++;
	len = WPA_GET_BE24(pos);
	pos += 3;
	left -= 4;

	if (len > left) {
		wpa_printf(MSG_DEBUG, "TLSv1: Mismatch in ServerKeyExchange "
			   "length (len=%lu != left=%lu)",
			   (unsigned long) len, (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	end = pos + len;

	if (type == TLS_HANDSHAKE_TYPE_CERTIFICATE_REQUEST)
		return tls_process_certificate_request(conn, ct, in_data,
						       in_len);
	if (type == TLS_HANDSHAKE_TYPE_SERVER_HELLO_DONE)
		return tls_process_server_hello_done(conn, ct, in_data,
						     in_len);
	if (type != TLS_HANDSHAKE_TYPE_SERVER_KEY_EXCHANGE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Received unexpected handshake "
			   "message %d (expected ServerKeyExchange/"
			   "CertificateRequest/ServerHelloDone)", type);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "TLSv1: Received ServerKeyExchange");

	if (!tls_server_key_exchange_allowed(conn)) {
		wpa_printf(MSG_DEBUG, "TLSv1: ServerKeyExchange not allowed "
			   "with the selected cipher suite");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	wpa_hexdump(MSG_DEBUG, "TLSv1: ServerKeyExchange", pos, len);
	suite = tls_get_cipher_suite(conn->rl.cipher_suite);
	if (suite && suite->key_exchange == TLS_KEY_X_DH_anon) {
		if (tlsv1_process_diffie_hellman(conn, pos, len) < 0) {
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
				  TLS_ALERT_DECODE_ERROR);
			return -1;
		}
	} else {
		wpa_printf(MSG_DEBUG, "TLSv1: UnexpectedServerKeyExchange");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	*in_len = end - in_data;

	conn->state = SERVER_CERTIFICATE_REQUEST;

	return 0;
}


static int tls_process_certificate_request(struct tlsv1_client *conn, u8 ct,
					   const u8 *in_data, size_t *in_len)
{
	const u8 *pos, *end;
	size_t left, len;
	u8 type;

	if (ct != TLS_CONTENT_TYPE_HANDSHAKE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected Handshake; "
			   "received content type 0x%x", ct);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	pos = in_data;
	left = *in_len;

	if (left < 4) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short CertificateRequest "
			   "(left=%lu)", (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	type = *pos++;
	len = WPA_GET_BE24(pos);
	pos += 3;
	left -= 4;

	if (len > left) {
		wpa_printf(MSG_DEBUG, "TLSv1: Mismatch in CertificateRequest "
			   "length (len=%lu != left=%lu)",
			   (unsigned long) len, (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	end = pos + len;

	if (type == TLS_HANDSHAKE_TYPE_SERVER_HELLO_DONE)
		return tls_process_server_hello_done(conn, ct, in_data,
						     in_len);
	if (type != TLS_HANDSHAKE_TYPE_CERTIFICATE_REQUEST) {
		wpa_printf(MSG_DEBUG, "TLSv1: Received unexpected handshake "
			   "message %d (expected CertificateRequest/"
			   "ServerHelloDone)", type);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "TLSv1: Received CertificateRequest");

	conn->certificate_requested = 1;

	*in_len = end - in_data;

	conn->state = SERVER_HELLO_DONE;

	return 0;
}


static int tls_process_server_hello_done(struct tlsv1_client *conn, u8 ct,
					 const u8 *in_data, size_t *in_len)
{
	const u8 *pos, *end;
	size_t left, len;
	u8 type;

	if (ct != TLS_CONTENT_TYPE_HANDSHAKE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected Handshake; "
			   "received content type 0x%x", ct);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	pos = in_data;
	left = *in_len;

	if (left < 4) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short ServerHelloDone "
			   "(left=%lu)", (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	type = *pos++;
	len = WPA_GET_BE24(pos);
	pos += 3;
	left -= 4;

	if (len > left) {
		wpa_printf(MSG_DEBUG, "TLSv1: Mismatch in ServerHelloDone "
			   "length (len=%lu != left=%lu)",
			   (unsigned long) len, (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}
	end = pos + len;

	if (type != TLS_HANDSHAKE_TYPE_SERVER_HELLO_DONE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Received unexpected handshake "
			   "message %d (expected ServerHelloDone)", type);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "TLSv1: Received ServerHelloDone");

	*in_len = end - in_data;

	conn->state = CLIENT_KEY_EXCHANGE;

	return 0;
}


static int tls_process_server_change_cipher_spec(struct tlsv1_client *conn,
						 u8 ct, const u8 *in_data,
						 size_t *in_len)
{
	const u8 *pos;
	size_t left;

	if (ct != TLS_CONTENT_TYPE_CHANGE_CIPHER_SPEC) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected ChangeCipherSpec; "
			   "received content type 0x%x", ct);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	pos = in_data;
	left = *in_len;

	if (left < 1) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short ChangeCipherSpec");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	if (*pos != TLS_CHANGE_CIPHER_SPEC) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected ChangeCipherSpec; "
			   "received data 0x%x", *pos);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "TLSv1: Received ChangeCipherSpec");
	if (tlsv1_record_change_read_cipher(&conn->rl) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to change read cipher "
			   "for record layer");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	*in_len = pos + 1 - in_data;

	conn->state = SERVER_FINISHED;

	return 0;
}


static int tls_process_server_finished(struct tlsv1_client *conn, u8 ct,
				       const u8 *in_data, size_t *in_len)
{
	const u8 *pos, *end;
	size_t left, len, hlen;
	u8 verify_data[TLS_VERIFY_DATA_LEN];
	u8 hash[MD5_MAC_LEN + SHA1_MAC_LEN];

	if (ct != TLS_CONTENT_TYPE_HANDSHAKE) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected Finished; "
			   "received content type 0x%x", ct);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	pos = in_data;
	left = *in_len;

	if (left < 4) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short record (left=%lu) for "
			   "Finished",
			   (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_DECODE_ERROR);
		return -1;
	}

	if (pos[0] != TLS_HANDSHAKE_TYPE_FINISHED) {
		wpa_printf(MSG_DEBUG, "TLSv1: Expected Finished; received "
			   "type 0x%x", pos[0]);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_UNEXPECTED_MESSAGE);
		return -1;
	}

	len = WPA_GET_BE24(pos + 1);

	pos += 4;
	left -= 4;

	if (len > left) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short buffer for Finished "
			   "(len=%lu > left=%lu)",
			   (unsigned long) len, (unsigned long) left);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_DECODE_ERROR);
		return -1;
	}
	end = pos + len;
	if (len != TLS_VERIFY_DATA_LEN) {
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected verify_data length "
			   "in Finished: %lu (expected %d)",
			   (unsigned long) len, TLS_VERIFY_DATA_LEN);
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_DECODE_ERROR);
		return -1;
	}
	wpa_hexdump(MSG_MSGDUMP, "TLSv1: verify_data in Finished",
		    pos, TLS_VERIFY_DATA_LEN);

	hlen = MD5_MAC_LEN;
	if (conn->verify_md5_server == NULL ||
	    crypto_hash_finish(conn->verify_md5_server, hash, &hlen) < 0) {
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		conn->verify_md5_server = NULL;
		crypto_hash_finish(conn->verify_sha1_server, NULL, NULL);
		conn->verify_sha1_server = NULL;
		return -1;
	}
	conn->verify_md5_server = NULL;
	hlen = SHA1_MAC_LEN;
	if (conn->verify_sha1_server == NULL ||
	    crypto_hash_finish(conn->verify_sha1_server, hash + MD5_MAC_LEN,
			       &hlen) < 0) {
		conn->verify_sha1_server = NULL;
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}
	conn->verify_sha1_server = NULL;

	if (tls_prf(conn->master_secret, TLS_MASTER_SECRET_LEN,
		    "server finished", hash, MD5_MAC_LEN + SHA1_MAC_LEN,
		    verify_data, TLS_VERIFY_DATA_LEN)) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to derive verify_data");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_DECRYPT_ERROR);
		return -1;
	}
	wpa_hexdump_key(MSG_DEBUG, "TLSv1: verify_data (server)",
			verify_data, TLS_VERIFY_DATA_LEN);

	if (memcmp(pos, verify_data, TLS_VERIFY_DATA_LEN) != 0) {
		wpa_printf(MSG_INFO, "TLSv1: Mismatch in verify_data");
		return -1;
	}

	wpa_printf(MSG_DEBUG, "TLSv1: Received Finished");

	*in_len = end - in_data;

	conn->state = (conn->session_resumed || conn->ticket) ?
		CHANGE_CIPHER_SPEC : ACK_FINISHED;

	return 0;
}


static int tls_derive_pre_master_secret(u8 *pre_master_secret)
{
	WPA_PUT_BE16(pre_master_secret, TLS_VERSION);
	if (os_get_random(pre_master_secret + 2,
			  TLS_PRE_MASTER_SECRET_LEN - 2))
		return -1;
	return 0;
}


static int tls_derive_keys(struct tlsv1_client *conn,
			   const u8 *pre_master_secret,
			   size_t pre_master_secret_len)
{
	u8 seed[2 * TLS_RANDOM_LEN];
	u8 key_block[TLS_MAX_KEY_BLOCK_LEN];
	u8 *pos;
	size_t key_block_len;

	if (pre_master_secret) {
		wpa_hexdump_key(MSG_MSGDUMP, "TLSv1: pre_master_secret",
				pre_master_secret, pre_master_secret_len);
		memcpy(seed, conn->client_random, TLS_RANDOM_LEN);
		memcpy(seed + TLS_RANDOM_LEN, conn->server_random,
		       TLS_RANDOM_LEN);
		if (tls_prf(pre_master_secret, pre_master_secret_len,
			    "master secret", seed, 2 * TLS_RANDOM_LEN,
			    conn->master_secret, TLS_MASTER_SECRET_LEN)) {
			wpa_printf(MSG_DEBUG, "TLSv1: Failed to derive "
				   "master_secret");
			return -1;
		}
		wpa_hexdump_key(MSG_MSGDUMP, "TLSv1: master_secret",
				conn->master_secret, TLS_MASTER_SECRET_LEN);
	}

	memcpy(seed, conn->server_random, TLS_RANDOM_LEN);
	memcpy(seed + TLS_RANDOM_LEN, conn->client_random, TLS_RANDOM_LEN);
	key_block_len = 2 * (conn->rl.hash_size + conn->rl.key_material_len +
			     conn->rl.iv_size);
	if (tls_prf(conn->master_secret, TLS_MASTER_SECRET_LEN,
		    "key expansion", seed, 2 * TLS_RANDOM_LEN,
		    key_block, key_block_len)) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to derive key_block");
		return -1;
	}
	wpa_hexdump_key(MSG_MSGDUMP, "TLSv1: key_block",
			key_block, key_block_len);

	pos = key_block;

	/* client_write_MAC_secret */
	memcpy(conn->rl.write_mac_secret, pos, conn->rl.hash_size);
	pos += conn->rl.hash_size;
	/* server_write_MAC_secret */
	memcpy(conn->rl.read_mac_secret, pos, conn->rl.hash_size);
	pos += conn->rl.hash_size;

	/* client_write_key */
	memcpy(conn->rl.write_key, pos, conn->rl.key_material_len);
	pos += conn->rl.key_material_len;
	/* server_write_key */
	memcpy(conn->rl.read_key, pos, conn->rl.key_material_len);
	pos += conn->rl.key_material_len;

	/* client_write_IV */
	memcpy(conn->rl.write_iv, pos, conn->rl.iv_size);
	pos += conn->rl.iv_size;
	/* server_write_IV */
	memcpy(conn->rl.read_iv, pos, conn->rl.iv_size);
	pos += conn->rl.iv_size;

	return 0;
}


static int tls_write_client_certificate(struct tlsv1_client *conn,
					u8 **msgpos, u8 *end)
{
	u8 *pos, *rhdr, *hs_start, *hs_length;
	size_t rlen;

	pos = *msgpos;

	wpa_printf(MSG_DEBUG, "TLSv1: Send Certificate");
	rhdr = pos;
	pos += TLS_RECORD_HEADER_LEN;

	/* opaque fragment[TLSPlaintext.length] */

	/* Handshake */
	hs_start = pos;
	/* HandshakeType msg_type */
	*pos++ = TLS_HANDSHAKE_TYPE_CERTIFICATE;
	/* uint24 length (to be filled) */
	hs_length = pos;
	pos += 3;
	/* body - Certificate */
	/* TODO: add certificate chain, if configured */
	WPA_PUT_BE24(pos, 0);
	pos += 3;

	WPA_PUT_BE24(hs_length, pos - hs_length - 3);

	if (tlsv1_record_send(&conn->rl, TLS_CONTENT_TYPE_HANDSHAKE,
			      rhdr, end - rhdr, pos - hs_start, &rlen) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to generate a record");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}
	pos = rhdr + rlen;

	tls_verify_hash_add(conn, hs_start, pos - hs_start);

	*msgpos = pos;

	return 0;
}


int tlsv1_key_x_anon_dh(struct tlsv1_client *conn, u8 **pos, u8 *end)
{
#ifdef EAP_FAST
	/* ClientDiffieHellmanPublic */
	u8 *csecret, *csecret_start, *dh_yc, *shared;
	size_t csecret_len, dh_yc_len, shared_len;

	csecret_len = conn->dh_p_len;
	csecret = malloc(csecret_len);
	if (csecret == NULL) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to allocate "
			   "memory for Yc (Diffie-Hellman)");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}
	if (os_get_random(csecret, csecret_len)) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to get random "
			   "data for Diffie-Hellman");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		free(csecret);
		return -1;
	}

	if (memcmp(csecret, conn->dh_p, csecret_len) > 0)
		csecret[0] = 0; /* make sure Yc < p */

	csecret_start = csecret;
	while (csecret_len > 1 && *csecret_start == 0) {
		csecret_start++;
		csecret_len--;
	}
	wpa_hexdump_key(MSG_DEBUG, "TLSv1: DH client's secret value",
			csecret_start, csecret_len);

	/* Yc = g^csecret mod p */
	dh_yc_len = conn->dh_p_len;
	dh_yc = malloc(dh_yc_len);
	if (dh_yc == NULL) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to allocate "
			   "memory for Diffie-Hellman");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		free(csecret);
		return -1;
	}
	crypto_mod_exp(conn->dh_g, conn->dh_g_len,
		       csecret_start, csecret_len,
		       conn->dh_p, conn->dh_p_len,
		       dh_yc, &dh_yc_len);

	wpa_hexdump(MSG_DEBUG, "TLSv1: DH Yc (client's public value)",
		    dh_yc, dh_yc_len);

	WPA_PUT_BE16(*pos, dh_yc_len);
	*pos += 2;
	if (*pos + dh_yc_len > end) {
		wpa_printf(MSG_DEBUG, "TLSv1: Not enough room in the "
			   "message buffer for Yc");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		free(csecret);
		free(dh_yc);
		return -1;
	}
	memcpy(*pos, dh_yc, dh_yc_len);
	*pos += dh_yc_len;
	free(dh_yc);

	shared_len = conn->dh_p_len;
	shared = malloc(shared_len);
	if (shared == NULL) {
		wpa_printf(MSG_DEBUG, "TLSv1: Could not allocate memory for "
			   "DH");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		free(csecret);
		return -1;
	}

	/* shared = Ys^csecret mod p */
	crypto_mod_exp(conn->dh_ys, conn->dh_ys_len,
		       csecret_start, csecret_len,
		       conn->dh_p, conn->dh_p_len,
		       shared, &shared_len);
	wpa_hexdump_key(MSG_DEBUG, "TLSv1: Shared secret from DH key exchange",
			shared, shared_len);

	memset(csecret_start, 0, csecret_len);
	free(csecret);
	if (tls_derive_keys(conn, shared, shared_len)) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to derive keys");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		free(shared);
		return -1;
	}
	memset(shared, 0, shared_len);
	free(shared);
	tlsv1_client_free_dh(conn);
	return 0;
#else /* EAP_FAST */
	tls_alert(conn, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_INTERNAL_ERROR);
	return -1;
#endif /* EAP_FAST */
}


static int tlsv1_key_x_rsa(struct tlsv1_client *conn, u8 **pos, u8 *end)
{
	u8 pre_master_secret[TLS_PRE_MASTER_SECRET_LEN];
	size_t clen;
	int res;

	if (tls_derive_pre_master_secret(pre_master_secret) < 0 ||
	    tls_derive_keys(conn, pre_master_secret,
			    TLS_PRE_MASTER_SECRET_LEN)) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to derive keys");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	/* EncryptedPreMasterSecret */
	if (conn->server_rsa_key == NULL) {
		wpa_printf(MSG_DEBUG, "TLSv1: No server RSA key to "
			   "use for encrypting pre-master secret");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	/* RSA encrypted value is encoded with PKCS #1 v1.5 block type 2. */
	*pos += 2;
	clen = end - *pos;
	res = crypto_public_key_encrypt_pkcs1_v15(
		conn->server_rsa_key,
		pre_master_secret, TLS_PRE_MASTER_SECRET_LEN,
		*pos, &clen);
	memset(pre_master_secret, 0, TLS_PRE_MASTER_SECRET_LEN);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: RSA encryption failed");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}
	WPA_PUT_BE16(*pos - 2, clen);
	wpa_hexdump(MSG_MSGDUMP, "TLSv1: Encrypted pre_master_secret",
		    *pos, clen);
	*pos += clen;

	return 0;
}


static int tls_write_client_key_exchange(struct tlsv1_client *conn,
					 u8 **msgpos, u8 *end)
{
	u8 *pos, *rhdr, *hs_start, *hs_length;
	size_t rlen;
	tls_key_exchange keyx;
	const struct tls_cipher_suite *suite;

	suite = tls_get_cipher_suite(conn->rl.cipher_suite);
	if (suite == NULL)
		keyx = TLS_KEY_X_NULL;
	else
		keyx = suite->key_exchange;

	pos = *msgpos;

	wpa_printf(MSG_DEBUG, "TLSv1: Send ClientKeyExchange");

	rhdr = pos;
	pos += TLS_RECORD_HEADER_LEN;

	/* opaque fragment[TLSPlaintext.length] */

	/* Handshake */
	hs_start = pos;
	/* HandshakeType msg_type */
	*pos++ = TLS_HANDSHAKE_TYPE_CLIENT_KEY_EXCHANGE;
	/* uint24 length (to be filled) */
	hs_length = pos;
	pos += 3;
	/* body - ClientKeyExchange */
	if (keyx == TLS_KEY_X_DH_anon) {
		if (tlsv1_key_x_anon_dh(conn, &pos, end) < 0)
			return -1;
	} else {
		if (tlsv1_key_x_rsa(conn, &pos, end) < 0)
			return -1;
	}

	WPA_PUT_BE24(hs_length, pos - hs_length - 3);

	if (tlsv1_record_send(&conn->rl, TLS_CONTENT_TYPE_HANDSHAKE,
			      rhdr, end - rhdr, pos - hs_start, &rlen) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to create a record");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}
	pos = rhdr + rlen;
	tls_verify_hash_add(conn, hs_start, pos - hs_start);

	*msgpos = pos;

	return 0;
}


static int tls_write_client_change_cipher_spec(struct tlsv1_client *conn,
					       u8 **msgpos, u8 *end)
{
	u8 *pos, *rhdr;
	size_t rlen;

	pos = *msgpos;

	wpa_printf(MSG_DEBUG, "TLSv1: Send ChangeCipherSpec");
	rhdr = pos;
	pos += TLS_RECORD_HEADER_LEN;
	*pos = TLS_CHANGE_CIPHER_SPEC;
	if (tlsv1_record_send(&conn->rl, TLS_CONTENT_TYPE_CHANGE_CIPHER_SPEC,
			      rhdr, end - rhdr, 1, &rlen) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to create a record");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	if (tlsv1_record_change_write_cipher(&conn->rl) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to set write cipher for "
			   "record layer");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	*msgpos = rhdr + rlen;

	return 0;
}


static int tls_write_client_finished(struct tlsv1_client *conn,
				     u8 **msgpos, u8 *end)
{
	u8 *pos, *rhdr, *hs_start, *hs_length;
	size_t rlen, hlen;
	u8 verify_data[TLS_VERIFY_DATA_LEN];
	u8 hash[MD5_MAC_LEN + SHA1_MAC_LEN];

	pos = *msgpos;

	wpa_printf(MSG_DEBUG, "TLSv1: Send Finished");

	/* Encrypted Handshake Message: Finished */

	hlen = MD5_MAC_LEN;
	if (conn->verify_md5_client == NULL ||
	    crypto_hash_finish(conn->verify_md5_client, hash, &hlen) < 0) {
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		conn->verify_md5_client = NULL;
		crypto_hash_finish(conn->verify_sha1_client, NULL, NULL);
		conn->verify_sha1_client = NULL;
		return -1;
	}
	conn->verify_md5_client = NULL;
	hlen = SHA1_MAC_LEN;
	if (conn->verify_sha1_client == NULL ||
	    crypto_hash_finish(conn->verify_sha1_client, hash + MD5_MAC_LEN,
			       &hlen) < 0) {
		conn->verify_sha1_client = NULL;
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}
	conn->verify_sha1_client = NULL;

	if (tls_prf(conn->master_secret, TLS_MASTER_SECRET_LEN,
		    "client finished", hash, MD5_MAC_LEN + SHA1_MAC_LEN,
		    verify_data, TLS_VERIFY_DATA_LEN)) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to generate verify_data");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}
	wpa_hexdump_key(MSG_DEBUG, "TLSv1: verify_data (client)",
			verify_data, TLS_VERIFY_DATA_LEN);

	rhdr = pos;
	pos += TLS_RECORD_HEADER_LEN;
	/* Handshake */
	hs_start = pos;
	/* HandshakeType msg_type */
	*pos++ = TLS_HANDSHAKE_TYPE_FINISHED;
	/* uint24 length (to be filled) */
	hs_length = pos;
	pos += 3;
	memcpy(pos, verify_data, TLS_VERIFY_DATA_LEN);
	pos += TLS_VERIFY_DATA_LEN;
	WPA_PUT_BE24(hs_length, pos - hs_length - 3);
	tls_verify_hash_add(conn, hs_start, pos - hs_start);

	if (tlsv1_record_send(&conn->rl, TLS_CONTENT_TYPE_HANDSHAKE,
			      rhdr, end - rhdr, pos - hs_start, &rlen) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to create a record");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	pos = rhdr + rlen;

	*msgpos = pos;

	return 0;
}


static u8 * tls_send_client_key_exchange(struct tlsv1_client *conn,
					 size_t *out_len)
{
	u8 *msg, *end, *pos;

	*out_len = 0;

	msg = malloc(1000);
	if (msg == NULL)
		return NULL;

	pos = msg;
	end = msg + 1000;

	if (conn->certificate_requested) {
		if (tls_write_client_certificate(conn, &pos, end) < 0) {
			free(msg);
			return NULL;
		}
	}

	if (tls_write_client_key_exchange(conn, &pos, end) < 0 ||
	    tls_write_client_change_cipher_spec(conn, &pos, end) < 0 ||
	    tls_write_client_finished(conn, &pos, end) < 0) {
		free(msg);
		return NULL;
	}

	*out_len = pos - msg;

	conn->state = SERVER_CHANGE_CIPHER_SPEC;

	return msg;
}


static u8 * tls_send_change_cipher_spec(struct tlsv1_client *conn,
					size_t *out_len)
{
	u8 *msg, *end, *pos;

	*out_len = 0;

	msg = malloc(1000);
	if (msg == NULL)
		return NULL;

	pos = msg;
	end = msg + 1000;

	if (tls_write_client_change_cipher_spec(conn, &pos, end) < 0 ||
	    tls_write_client_finished(conn, &pos, end) < 0) {
		free(msg);
		return NULL;
	}

	*out_len = pos - msg;

	wpa_printf(MSG_DEBUG, "TLSv1: Session resumption completed "
		   "successfully");
	conn->state = ESTABLISHED;

	return msg;
}


static int tlsv1_client_process_handshake(struct tlsv1_client *conn, u8 ct,
					  const u8 *buf, size_t *len)
{
	if (ct == TLS_CONTENT_TYPE_HANDSHAKE && *len >= 4 &&
	    buf[0] == TLS_HANDSHAKE_TYPE_HELLO_REQUEST) {
		size_t hr_len = WPA_GET_BE24(buf + 1);
		if (hr_len > *len - 4) {
			wpa_printf(MSG_DEBUG, "TLSv1: HelloRequest underflow");
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
				  TLS_ALERT_DECODE_ERROR);
			return -1;
		}
		wpa_printf(MSG_DEBUG, "TLSv1: Ignored HelloRequest");
		*len = 4 + hr_len;
		return 0;
	}

	switch (conn->state) {
	case SERVER_HELLO:
		if (tls_process_server_hello(conn, ct, buf, len))
			return -1;
		break;
	case SERVER_CERTIFICATE:
		if (tls_process_certificate(conn, ct, buf, len))
			return -1;
		break;
	case SERVER_KEY_EXCHANGE:
		if (tls_process_server_key_exchange(conn, ct, buf, len))
			return -1;
		break;
	case SERVER_CERTIFICATE_REQUEST:
		if (tls_process_certificate_request(conn, ct, buf, len))
			return -1;
		break;
	case SERVER_HELLO_DONE:
		if (tls_process_server_hello_done(conn, ct, buf, len))
			return -1;
		break;
	case SERVER_CHANGE_CIPHER_SPEC:
		if (tls_process_server_change_cipher_spec(conn, ct, buf, len))
			return -1;
		break;
	case SERVER_FINISHED:
		if (tls_process_server_finished(conn, ct, buf, len))
			return -1;
		break;
	default:
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected state %d "
			   "while processing received message",
			   conn->state);
		return -1;
	}

	if (ct == TLS_CONTENT_TYPE_HANDSHAKE)
		tls_verify_hash_add(conn, buf, *len);

	return 0;
}


u8 * tlsv1_client_handshake(struct tlsv1_client *conn,
			    const u8 *in_data, size_t in_len,
			    size_t *out_len)
{
	const u8 *pos, *end;
	u8 *msg = NULL, *in_msg, *in_pos, *in_end, alert, ct;
	size_t in_msg_len;

	if (conn->state == CLIENT_HELLO) {
		if (in_len)
			return NULL;
		return tls_send_client_hello(conn, out_len);
	}

	if (in_data == NULL || in_len == 0)
		return NULL;

	pos = in_data;
	end = in_data + in_len;
	in_msg = malloc(in_len);
	if (in_msg == NULL)
		return NULL;

	/* Each received packet may include multiple records */
	while (pos < end) {
		in_msg_len = in_len;
		if (tlsv1_record_receive(&conn->rl, pos, end - pos,
					 in_msg, &in_msg_len, &alert)) {
			wpa_printf(MSG_DEBUG, "TLSv1: Processing received "
				   "record failed");
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL, alert);
			goto failed;
		}
		ct = pos[0];

		in_pos = in_msg;
		in_end = in_msg + in_msg_len;

		/* Each received record may include multiple messages of the
		 * same ContentType. */
		while (in_pos < in_end) {
			in_msg_len = in_end - in_pos;
			if (tlsv1_client_process_handshake(conn, ct, in_pos,
							   &in_msg_len) < 0)
				goto failed;
			in_pos += in_msg_len;
		}

		pos += TLS_RECORD_HEADER_LEN + WPA_GET_BE16(pos + 3);
	}

	free(in_msg);
	in_msg = NULL;

	switch (conn->state) {
	case CLIENT_KEY_EXCHANGE:
		msg = tls_send_client_key_exchange(conn, out_len);
		break;
	case CHANGE_CIPHER_SPEC:
		msg = tls_send_change_cipher_spec(conn, out_len);
		break;
	case ACK_FINISHED:
		wpa_printf(MSG_DEBUG, "TLSv1: Handshake completed "
			   "successfully");
		conn->state = ESTABLISHED;
		/* Need to return something to get final TLS ACK. */
		msg = malloc(1);
		*out_len = 0;
		break;
	default:
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected state %d while "
			   "generating reply", conn->state);
		break;
	}

failed:
	free(in_msg);
	if (conn->alert_level) {
		conn->state = FAILED;
		free(msg);
		msg = tls_send_alert(conn, conn->alert_level,
				     conn->alert_description, out_len);
	}

	return msg;
}


int tlsv1_client_encrypt(struct tlsv1_client *conn,
			 const u8 *in_data, size_t in_len,
			 u8 *out_data, size_t out_len)
{
	size_t rlen;

	wpa_hexdump_key(MSG_MSGDUMP, "TLSv1: Plaintext AppData",
			in_data, in_len);

	memcpy(out_data + TLS_RECORD_HEADER_LEN, in_data, in_len);

	if (tlsv1_record_send(&conn->rl, TLS_CONTENT_TYPE_APPLICATION_DATA,
			      out_data, out_len, in_len, &rlen) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to create a record");
		tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
			  TLS_ALERT_INTERNAL_ERROR);
		return -1;
	}

	return rlen;
}


int tlsv1_client_decrypt(struct tlsv1_client *conn,
			 const u8 *in_data, size_t in_len,
			 u8 *out_data, size_t out_len)
{
	const u8 *in_end, *pos;
	int res;
	u8 alert, *out_end, *out_pos;
	size_t olen;

	pos = in_data;
	in_end = in_data + in_len;
	out_pos = out_data;
	out_end = out_data + out_len;

	while (pos < in_end) {
		if (pos[0] != TLS_CONTENT_TYPE_APPLICATION_DATA) {
			wpa_printf(MSG_DEBUG, "TLSv1: Unexpected content type "
				   "0x%x", pos[0]);
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
				  TLS_ALERT_UNEXPECTED_MESSAGE);
			return -1;
		}

		olen = out_end - out_pos;
		res = tlsv1_record_receive(&conn->rl, pos, in_end - pos,
					   out_pos, &olen, &alert);
		if (res < 0) {
			wpa_printf(MSG_DEBUG, "TLSv1: Record layer processing "
				   "failed");
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL, alert);
			return -1;
		}
		out_pos += olen;
		if (out_pos > out_end) {
			wpa_printf(MSG_DEBUG, "TLSv1: Buffer not large enough "
				   "for processing the received record");
			tls_alert(conn, TLS_ALERT_LEVEL_FATAL,
				  TLS_ALERT_INTERNAL_ERROR);
			return -1;
		}

		pos += TLS_RECORD_HEADER_LEN + WPA_GET_BE16(pos + 3);
	}

	return out_pos - out_data;
}


int tlsv1_client_global_init(void)
{
	return crypto_global_init();
}


void tlsv1_client_global_deinit(void)
{
	crypto_global_deinit();
}


static void tlsv1_client_free_verify_hashes(struct tlsv1_client *conn)
{
	crypto_hash_finish(conn->verify_md5_client, NULL, NULL);
	crypto_hash_finish(conn->verify_md5_server, NULL, NULL);
	crypto_hash_finish(conn->verify_sha1_client, NULL, NULL);
	crypto_hash_finish(conn->verify_sha1_server, NULL, NULL);
	conn->verify_md5_client = NULL;
	conn->verify_md5_server = NULL;
	conn->verify_sha1_client = NULL;
	conn->verify_sha1_server = NULL;
}


static int tlsv1_client_init_verify_hashes(struct tlsv1_client *conn)
{
	conn->verify_md5_client = crypto_hash_init(CRYPTO_HASH_ALG_MD5, NULL,
						   0);
	conn->verify_md5_server = crypto_hash_init(CRYPTO_HASH_ALG_MD5, NULL,
						   0);
	conn->verify_sha1_client = crypto_hash_init(CRYPTO_HASH_ALG_SHA1, NULL,
						    0);
	conn->verify_sha1_server = crypto_hash_init(CRYPTO_HASH_ALG_SHA1, NULL,
						    0);
	if (conn->verify_md5_client == NULL ||
	    conn->verify_md5_server == NULL ||
	    conn->verify_sha1_client == NULL ||
	    conn->verify_sha1_server == NULL) {
		tlsv1_client_free_verify_hashes(conn);
		return -1;
	}
	return 0;
}


struct tlsv1_client * tlsv1_client_init(void)
{
	struct tlsv1_client *conn;
	size_t count;
	u16 *suites;

	conn = wpa_zalloc(sizeof(*conn));
	if (conn == NULL)
		return NULL;

	conn->state = CLIENT_HELLO;

	if (tlsv1_client_init_verify_hashes(conn) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to initialize verify "
			   "hash");
		free(conn);
		return NULL;
	}

	count = 0;
	suites = conn->cipher_suites;
	suites[count++] = TLS_RSA_WITH_AES_256_CBC_SHA;
	suites[count++] = TLS_RSA_WITH_AES_128_CBC_SHA;
	suites[count++] = TLS_RSA_WITH_3DES_EDE_CBC_SHA;
	suites[count++] = TLS_RSA_WITH_RC4_128_SHA;
	suites[count++] = TLS_RSA_WITH_RC4_128_MD5;
	conn->num_cipher_suites = count;

	return conn;
}


void tlsv1_client_deinit(struct tlsv1_client *conn)
{
	crypto_public_key_free(conn->server_rsa_key);
	tlsv1_record_set_cipher_suite(&conn->rl, TLS_NULL_WITH_NULL_NULL);
	tlsv1_record_change_write_cipher(&conn->rl);
	tlsv1_record_change_read_cipher(&conn->rl);
	tlsv1_client_free_verify_hashes(conn);
	free(conn->client_hello_ext);
	tlsv1_client_free_dh(conn);
	free(conn);
}


int tlsv1_client_established(struct tlsv1_client *conn)
{
	return conn->state == ESTABLISHED;
}


int tlsv1_client_prf(struct tlsv1_client *conn, const char *label,
		     int server_random_first, u8 *out, size_t out_len)
{
	u8 seed[2 * TLS_RANDOM_LEN];

	if (conn->state != ESTABLISHED)
		return -1;

	if (server_random_first) {
		memcpy(seed, conn->server_random, TLS_RANDOM_LEN);
		memcpy(seed + TLS_RANDOM_LEN, conn->client_random,
		       TLS_RANDOM_LEN);
	} else {
		memcpy(seed, conn->client_random, TLS_RANDOM_LEN);
		memcpy(seed + TLS_RANDOM_LEN, conn->server_random,
		       TLS_RANDOM_LEN);
	}

	return tls_prf(conn->master_secret, TLS_MASTER_SECRET_LEN,
		       label, seed, 2 * TLS_RANDOM_LEN, out, out_len);
}


int tlsv1_client_get_cipher(struct tlsv1_client *conn, char *buf,
			    size_t buflen)
{
	char *cipher;

	switch (conn->rl.cipher_suite) {
	case TLS_RSA_WITH_RC4_128_MD5:
		cipher = "RC4-MD5";
		break;
	case TLS_RSA_WITH_RC4_128_SHA:
		cipher = "RC4-SHA";
		break;
	case TLS_RSA_WITH_DES_CBC_SHA:
		cipher = "DES-CBC-SHA";
		break;
	case TLS_RSA_WITH_3DES_EDE_CBC_SHA:
		cipher = "DES-CBC3-SHA";
		break;
	default:
		return -1;
	}

	snprintf(buf, buflen, "%s", cipher);
	return 0;
}


int tlsv1_client_shutdown(struct tlsv1_client *conn)
{
	conn->state = CLIENT_HELLO;

	tlsv1_client_free_verify_hashes(conn);
	if (tlsv1_client_init_verify_hashes(conn) < 0) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to re-initialize verify "
			   "hash");
		return -1;
	}

	tlsv1_record_set_cipher_suite(&conn->rl, TLS_NULL_WITH_NULL_NULL);
	tlsv1_record_change_write_cipher(&conn->rl);
	tlsv1_record_change_read_cipher(&conn->rl);

	conn->certificate_requested = 0;
	crypto_public_key_free(conn->server_rsa_key);
	conn->server_rsa_key = NULL;
	conn->session_resumed = 0;

	return 0;
}


int tlsv1_client_resumed(struct tlsv1_client *conn)
{
	return !!conn->session_resumed;
}


int tlsv1_client_hello_ext(struct tlsv1_client *conn, int ext_type,
			   const u8 *data, size_t data_len)
{
	u8 *pos;

	conn->ticket = 0;
	free(conn->client_hello_ext);
	conn->client_hello_ext = NULL;
	conn->client_hello_ext_len = 0;

	if (data == NULL || data_len == 0)
		return 0;

	pos = conn->client_hello_ext = malloc(6 + data_len);
	if (pos == NULL)
		return -1;

	WPA_PUT_BE16(pos, 4 + data_len);
	pos += 2;
	WPA_PUT_BE16(pos, ext_type);
	pos += 2;
	WPA_PUT_BE16(pos, data_len);
	pos += 2;
	memcpy(pos, data, data_len);
	conn->client_hello_ext_len = 6 + data_len;

	if (ext_type == TLS_EXT_PAC_OPAQUE) {
		conn->ticket = 1;
		wpa_printf(MSG_DEBUG, "TLSv1: Using session ticket");
	}

	return 0;
}


int tlsv1_client_get_keys(struct tlsv1_client *conn, struct tls_keys *keys)
{
	memset(keys, 0, sizeof(*keys));
	if (conn->state == CLIENT_HELLO)
		return -1;

	keys->client_random = conn->client_random;
	keys->client_random_len = TLS_RANDOM_LEN;

	if (conn->state != SERVER_HELLO) {
		keys->server_random = conn->server_random;
		keys->server_random_len = TLS_RANDOM_LEN;
		keys->master_key = conn->master_secret;
		keys->master_key_len = TLS_MASTER_SECRET_LEN;
	}

	return 0;
}


int tlsv1_client_set_master_key(struct tlsv1_client *conn,
				const u8 *key, size_t key_len)
{
	if (key_len > TLS_MASTER_SECRET_LEN)
		return -1;
	wpa_hexdump_key(MSG_MSGDUMP, "TLSv1: master_secret from session "
			"ticket", key, key_len);
	memcpy(conn->master_secret, key, key_len);
	conn->ticket_key = 1;

	return 0;
}


int tlsv1_client_get_keyblock_size(struct tlsv1_client *conn)
{
	if (conn->state == CLIENT_HELLO || conn->state == SERVER_HELLO)
		return -1;

	return 2 * (conn->rl.hash_size + conn->rl.key_material_len +
		    conn->rl.iv_size);
}


int tlsv1_client_set_cipher_list(struct tlsv1_client *conn, u8 *ciphers)
{
#ifdef EAP_FAST
	size_t count;
	u16 *suites;

	/* TODO: implement proper configuration of cipher suites */
	if (ciphers[0] == TLS_CIPHER_ANON_DH_AES128_SHA) {
		count = 0;
		suites = conn->cipher_suites;
		suites[count++] = TLS_DH_anon_WITH_AES_256_CBC_SHA;
		suites[count++] = TLS_DH_anon_WITH_AES_128_CBC_SHA;
		suites[count++] = TLS_DH_anon_WITH_3DES_EDE_CBC_SHA;
		suites[count++] = TLS_DH_anon_WITH_RC4_128_MD5;
		suites[count++] = TLS_DH_anon_WITH_DES_CBC_SHA;
		conn->num_cipher_suites = count;
	}

	return 0;
#else /* EAP_FAST */
	return -1;
#endif /* EAP_FAST */
}
