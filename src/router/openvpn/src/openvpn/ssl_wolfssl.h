/*
 *  OpenVPN -- An application to securely tunnel IP networks
 *             over a single TCP/UDP port, with support for SSL/TLS-based
 *             session authentication and key exchange,
 *             packet encryption, packet authentication, and
 *             packet compression.
 *
 *  Copyright (C) 2002-2019 OpenVPN Inc <sales@openvpn.net>
 *  Copyright (C) 2010-2019 Fox Crypto B.V. <openvpn@fox-it.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file Control Channel wolfSSL Backend
 */

#ifndef SSL_WOLFSSL_H_
#define SSL_WOLFSSL_H_

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/pkcs12.h>
//#include <wolfssl/openssl/ec.h>
#include <wolfssl/ssl.h>

#define TLS1_1_VERSION                  0x0302
#define TLS1_2_VERSION                  0x0303
#define TLS1_3_VERSION                  0x0304

/* The list_buffer_t structure malloc's increments of BUCKET_BUF_LEN buckets */
#define BUCKET_BUF_LEN (1024*5)

/*
 * The len and offset members refer to the length and offset within a bucket.
 * Each bucket can hold up to BUCKET_BUF_LEN data.
 */
struct bucket_t
{
    uint32_t len;
    uint32_t offset;
    struct bucket_t* next;
    uint8_t buf[BUCKET_BUF_LEN];
};

/*
 * The buffer uses a list of buckets to hold data. This way the optimal amount
 * of space is used (buckets are malloc'ed and free'd accordingly). The len
 * member tracks the overall length of available data across all buckets.
 * The granularity of BUCKET_BUF_LEN avoids malloc'ing too much memory or calling
 * malloc too often.
 */
struct list_buffer_t
{
    uint32_t len;
    struct bucket_t* first;
    struct bucket_t* last;
};

/**
 * Structure that wraps the TLS context. Contents differ depending on the
 * SSL library used.
 */
struct tls_root_ctx
{
    WOLFSSL_CTX *ctx;
    time_t crl_last_mtime;
    off_t crl_last_size;
};

struct key_state_ssl
{
    WOLFSSL *ssl;
    struct list_buffer_t *send_buf;
    struct list_buffer_t *recv_buf;
    struct tls_session *session;
};

#endif /* SSL_WOLFSSL_H_ */
