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

#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#include "syshead.h"

#if defined(ENABLE_CRYPTO_WOLFSSL)

#include "errlevel.h"
#include "buffer.h"
#include "misc.h"
#include "manage.h"
#include "memdbg.h"
#include "ssl_backend.h"
#include "ssl_common.h"
#include "ssl_verify_wolfssl.h"
#include "base64.h"

/*
 *
 * Functions used in ssl.c which must be implemented by the backend SSL library
 *
 */

void tls_init_lib(void)
{
    int ret;
    if ((ret = wolfSSL_Init()) != SSL_SUCCESS)
    {
        msg(M_FATAL, "wolfSSL_Init failed with Errno: %d", ret);
    }
}

void tls_free_lib(void)
{
    int ret;
    if ((ret = wolfSSL_Cleanup()) != SSL_SUCCESS)
    {
        msg(M_FATAL, "wolfSSL_Cleanup failed with Errno: %d", ret);
    }
}

void tls_clear_error(void)
{
    wolfSSL_ERR_clear_error();
}

int tls_version_max(void)
{
#ifdef WOLFSSL_TLS13
    return TLS_VER_1_3;
#endif
    return TLS_VER_1_2;
}

void tls_ctx_server_new(struct tls_root_ctx *ctx)
{
    ASSERT(NULL != ctx);

    ctx->ctx = wolfSSL_CTX_new(wolfSSLv23_server_method());
    check_malloc_return(ctx->ctx);
}

void tls_ctx_client_new(struct tls_root_ctx *ctx)
{
    ASSERT(NULL != ctx);

    ctx->ctx = wolfSSL_CTX_new(wolfSSLv23_client_method());
    check_malloc_return(ctx->ctx);
}

void tls_ctx_free(struct tls_root_ctx *ctx)
{
    ASSERT(NULL != ctx);
    if (NULL != ctx->ctx)
    {
        wolfSSL_CTX_free(ctx->ctx);
        ctx->ctx = NULL;
    }
}

bool tls_ctx_initialised(struct tls_root_ctx *ctx)
{
    ASSERT(NULL != ctx);
    return NULL != ctx->ctx;
}

static void info_callback(const WOLFSSL* ssl, int type, int val)
{
    if (type & SSL_CB_LOOP)
    {
        dmsg(D_HANDSHAKE_VERBOSE, "SSL state (%s): %s",
                type & SSL_ST_CONNECT ? "connect" :
                type & SSL_ST_ACCEPT ? "accept" : "undefined",
                wolfSSL_state_string_long(ssl));
    }
    else if (type & SSL_CB_ALERT)
    {
        dmsg(D_HANDSHAKE_VERBOSE, "SSL alert (%s): %s",
                type & SSL_CB_READ ? "read" : "write",
                wolfSSL_alert_type_string_long(val));
    }
}

bool tls_ctx_set_options(struct tls_root_ctx *ctx, unsigned int ssl_flags)
{
    int ret = SSL_SUCCESS;
    int verify_flags = WOLFSSL_VERIFY_PEER
            | WOLFSSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    ASSERT(NULL != ctx);

    switch ((ssl_flags >> SSLF_TLS_VERSION_MIN_SHIFT)
            & SSLF_TLS_VERSION_MIN_MASK)
    {
    case TLS_VER_1_3:
        ret = wolfSSL_CTX_SetMinVersion(ctx->ctx, WOLFSSL_TLSV1_3);
        break;
    case TLS_VER_1_2:
        ret = wolfSSL_CTX_SetMinVersion(ctx->ctx, WOLFSSL_TLSV1_2);
        break;
    case TLS_VER_1_1:
        ret = wolfSSL_CTX_SetMinVersion(ctx->ctx, WOLFSSL_TLSV1_1);
        break;
    case TLS_VER_1_0:
        ret = wolfSSL_CTX_SetMinVersion(ctx->ctx, WOLFSSL_TLSV1);
        break;
    case TLS_VER_UNSPEC:
        break;
    default:
        msg(M_FATAL, "Unidentified minimum TLS version");
    }

    if (ret != SSL_SUCCESS)
    {
        msg(M_FATAL, "wolfSSL_CTX_SetMinVersion failed");
    }

    switch ((ssl_flags >> SSLF_TLS_VERSION_MAX_SHIFT)
            & SSLF_TLS_VERSION_MAX_MASK)
    {
    case TLS_VER_1_0:
        wolfSSL_CTX_set_options(ctx->ctx, SSL_OP_NO_TLSv1_1);
        /* no break */
    case TLS_VER_1_1:
        wolfSSL_CTX_set_options(ctx->ctx, SSL_OP_NO_TLSv1_2);
        /* no break */
    case TLS_VER_1_2:
        wolfSSL_CTX_set_options(ctx->ctx, SSL_OP_NO_TLSv1_3);
        /* no break */
    case TLS_VER_1_3:
    case TLS_VER_UNSPEC:
        break;
    default:
        msg(M_FATAL, "Unidentified maximum TLS version");
    }

    wolfSSL_CTX_set_session_cache_mode(ctx->ctx, WOLFSSL_SESS_CACHE_OFF);
    wolfSSL_CTX_set_default_passwd_cb(ctx->ctx, pem_password_callback);
    wolfSSL_CTX_set_info_callback(ctx->ctx, info_callback);

    /* Require peer certificate verification */
#if P2MP_SERVER
    if (ssl_flags & SSLF_CLIENT_CERT_NOT_REQUIRED)
    {
        verify_flags = WOLFSSL_VERIFY_NONE;
    }
    else if (ssl_flags & SSLF_CLIENT_CERT_OPTIONAL)
    {
        verify_flags = WOLFSSL_VERIFY_PEER;
    }
#endif

    wolfSSL_CTX_set_verify(ctx->ctx, verify_flags, &verify_callback);

    return true;
}

void tls_ctx_restrict_ciphers(struct tls_root_ctx *ctx, const char *ciphers)
{
    if (ciphers == NULL)
    {
        return;
    }

    if (wolfSSL_CTX_set_cipher_list(ctx->ctx, ciphers) != SSL_SUCCESS)
    {
        msg(M_FATAL, "Failed to set ciphers: %s", ciphers);
    }
}

void tls_ctx_restrict_ciphers_tls13(struct tls_root_ctx *ctx,
        const char *ciphers)
{
    if (ciphers == NULL)
    {
        return;
    }

    if (wolfSSL_CTX_set_cipher_list(ctx->ctx, ciphers) != SSL_SUCCESS)
    {
        msg(M_FATAL, "Failed to set ciphers: %s", ciphers);
    }
}

void tls_ctx_set_cert_profile(struct tls_root_ctx *ctx, const char *profile)
{
    if (profile)
    {
        msg(M_WARN, "WARNING: wolfSSL does not support --tls-cert-profile"
                ", ignoring user-set profile: '%s'", profile);
    }
}

void tls_ctx_check_cert_time(const struct tls_root_ctx *ctx)
{
    /*
     * This is verified during loading of certificate.
     */
}

void
tls_ctx_load_dh_params(struct tls_root_ctx *ctx, const char *dh_file,
                       bool dh_file_inline)
{
    int dh_len, ret;

    ASSERT(ctx != NULL);

    if (dh_file_inline)
    {
        /* Parameters in memory */
        if ((dh_len = strlen(dh_file)) == 0)
        {
            msg(M_FATAL, "Empty DH parameters passed.");
        }

        if ((ret = wolfSSL_CTX_SetTmpDH_buffer(ctx->ctx,
                (uint8_t*) dh_file, dh_len,
                SSL_FILETYPE_PEM)) != SSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_SetTmpDH_buffer failed with Errno: %d",
                    ret);
        }
    }
    else
    {
        /* Parameters in file */
        if ((ret = wolfSSL_CTX_SetTmpDH_file(ctx->ctx, dh_file,
        SSL_FILETYPE_PEM)) != SSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_SetTmpDH_file failed with Errno: %d",
                    ret);
        }
    }
}

void tls_ctx_load_ecdh_params(struct tls_root_ctx *ctx, const char *curve_name)
{
    int nid;
    WOLFSSL_EC_KEY* ecdh;

    if (curve_name == NULL)
    {
        return;
    }

    msg(D_TLS_DEBUG, "Using user specified ECDH curve (%s)", curve_name);

    if ((nid = wc_ecc_get_curve_id_from_name(curve_name)) < 0)
    {
        msg(M_FATAL, "Unknown curve name: %s", curve_name);
    }

    if (!(ecdh = wolfSSL_EC_KEY_new_by_curve_name(nid)))
    {
        msg(M_FATAL, "wolfSSL_EC_KEY_new_by_curve_name failed");
    }

    if (wolfSSL_SSL_CTX_set_tmp_ecdh(ctx->ctx, ecdh) != WOLFSSL_SUCCESS)
    {
        wolfSSL_EC_KEY_free(ecdh);
        msg(M_FATAL, "wolfSSL_SSL_CTX_set_tmp_ecdh failed");
    }

    wolfSSL_EC_KEY_free(ecdh);
}

int
tls_ctx_load_pkcs12(struct tls_root_ctx *ctx, const char *pkcs12_file,
                    bool pkcs12_file_inline, bool load_ca_file)
{
    int err, i, ret = 1,fd, size;
    uint32_t pkcs12_len;
    struct gc_arena gc = gc_new();
    struct buffer buf;
    WC_PKCS12* pkcs12 = wc_PKCS12_new();
    WOLFSSL_EVP_PKEY* pkey;
    WOLFSSL_X509* cert;
    const uint8_t* cert_der;
    int cert_der_len;
    WOLFSSL_X509_STORE* store;
    WOLF_STACK_OF(WOLFSSL_X509)* ca;
    char password[256];

    ASSERT(ctx != NULL);

    if (pkcs12_file_inline)
    {
        /* PKCS12 in memory */
        if ((pkcs12_len = strlen(pkcs12_file)) == 0)
        {
            msg(M_FATAL, "Empty pkcs12 parameters passed.");
        }

        /* DER length will be less than PEM length */
        buf = alloc_buf_gc(pkcs12_len, &gc);
        if (!buf_valid(&buf))
        {
            msg(M_FATAL, "Error allocating %d bytes for pkcs12 buffer",
                    pkcs12_len);
        }

        if ((err = Base64_Decode((uint8_t*) pkcs12_file, pkcs12_len,
                BPTR(&buf), &pkcs12_len)) != 0)
        {
            msg(M_FATAL, "Base64_Decode failed with Errno: %d", err);
        }
        buf.len = pkcs12_len;
    }
    else
    {
        /* PKCS12 in file */
//        buf = buffer_read_from_file(pkcs12_file, &gc);

        buf = alloc_buf_gc(2048, &gc);
        fd = platform_open(pkcs12_file, O_RDONLY, 0);
        if (fd == -1)
        {
            msg(M_ERR, "Cannot open key file '%s'", pkcs12_file);
        }
        size = read(fd, buf.data, buf.capacity);
        if (size < 0)
        {
            msg(M_FATAL, "Read error on key file ('%s')", pkcs12_file);
        }
        if (size == buf.capacity)
        {
            msg(M_FATAL, "Key file ('%s') can be a maximum of %d bytes", pkcs12_file, (int)buf.capacity);
        }
        close(fd);


        if (!buf_valid(&buf))
        {
            msg(M_FATAL, "Read error on pkcs12 file ('%s')", pkcs12_file);
        }


    }

    if ((err = wc_d2i_PKCS12(BPTR(&buf), BLEN(&buf), pkcs12)) != 0)
    {
        msg(M_FATAL, "wc_d2i_PKCS12 failed. Errno: %d", err);
    }

    if (wolfSSL_PKCS12_parse(pkcs12, "", &pkey, &cert, &ca) != WOLFSSL_SUCCESS)
    {
        pem_password_callback(password, sizeof(password) - 1, 0, NULL);
        if (wolfSSL_PKCS12_parse(pkcs12, password, &pkey, &cert, &ca)
                != WOLFSSL_SUCCESS)
        {
            msg(M_INFO,
                    "wolfSSL_PKCS12_parse failed. wolfSSL only supports PKCS "
                            "data encrypted using SHA1 with 128 bit RC4 and SHA1 with "
                            "DES3-CBC. Please check that the certificate is using these "
                            "encryption algorithms. When compiling a certificate with "
                            "OpenSSL use the -descert option to use the appropriate "
                            "algorithm.");
            goto cleanup;
        }
    }

    if (pkey)
    {
        if (wolfSSL_CTX_use_PrivateKey(ctx->ctx, pkey) != WOLFSSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_use_PrivateKey failed.");
        }
    }

    if (cert)
    {
        if (!(cert_der = wolfSSL_X509_get_der(cert, &cert_der_len)))
        {
            msg(M_FATAL, "wolfSSL_X509_get_der failed.");
        }
        if ((err = wolfSSL_CTX_use_certificate_buffer(ctx->ctx, cert_der,
                cert_der_len, WOLFSSL_FILETYPE_ASN1)) != SSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_use_certificate_buffer failed. Errno: %d",
                    err);
        }
    }

    ASSERT(store = wolfSSL_CTX_get_cert_store(ctx->ctx));

    for (i = 0; i < wolfSSL_sk_GENERAL_NAME_num(ca); i++)
    {
        WOLFSSL_X509* x509 = wolfSSL_sk_X509_value(ca, i);
        if (wolfSSL_X509_STORE_add_cert(store, x509) != WOLFSSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_X509_STORE_add_cert failed.");
        }
    }

    ret = 0;

    cleanup: if (pkey)
    {
        wolfSSL_EVP_PKEY_free(pkey);
    }
    if (cert)
    {
        wolfSSL_X509_free(cert);
    }
    if (ca)
    {
        wolfSSL_sk_X509_free(ca);
    }
    if (pkcs12)
    {
        wc_PKCS12_free(pkcs12);
    }
    gc_free(&gc);

    return ret;
}

#ifdef ENABLE_CRYPTOAPI
void
tls_ctx_load_cryptoapi(struct tls_root_ctx *ctx, const char *cryptoapi_cert)
{
    msg(M_FATAL, "Windows CryptoAPI is not yet supported for wolfSSL.");
}
#endif /* ENABLE_CRYPTOAPI */

int
tls_ctx_use_external_private_key(struct tls_root_ctx *ctx,
                                 const char *cert_file, const char *cert_file_inline)
{
    ASSERT(NULL != ctx);

    tls_ctx_load_cert_file(ctx, cert_file, cert_file_inline);
    if (ctx->ctx == NULL)
	return 1;
    return 0;
}

void
tls_ctx_load_cert_file(struct tls_root_ctx *ctx, const char *cert_file,
                       bool cert_inline)
{
    int ret;
    int cert_len;
    ASSERT(ctx != NULL);

    if (cert_inline)
    {
        /* Certificate in memory */
        if ((cert_len = strlen(cert_file)) == 0)
        {
            msg(M_FATAL, "Empty certificate passed.");
            return;
        }
        /*
         * Load certificate.
         */
        if ((ret = wolfSSL_CTX_use_certificate_chain_buffer(ctx->ctx,
                (uint8_t*) cert_file, cert_len)) != SSL_SUCCESS)
        {
            msg(M_FATAL,
                    "wolfSSL_CTX_use_certificate_buffer failed with Errno: %d",
                    ret);
            return;
        }
        /*
         * Load any additional certificates.
         */
        if ((ret = wolfSSL_CTX_load_verify_buffer(ctx->ctx,
                (uint8_t*) cert_file, cert_len,
                SSL_FILETYPE_PEM)) != SSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_load_verify_buffer failed with Errno: %d",
                    ret);
            return;
        }
    }
    else
    {
        /* Certificate in file */
        /*
         * Load certificate.
         */
        if ((ret = wolfSSL_CTX_use_certificate_chain_file(ctx->ctx, cert_file))
                != SSL_SUCCESS)
        {
            msg(M_FATAL,
                    "wolfSSL_CTX_use_certificate_chain_file failed with Errno: %d",
                    ret);
            return;
        }
        /*
         * Load any additional certificates.
         */
        if ((ret = wolfSSL_CTX_load_verify_locations(ctx->ctx, cert_file, NULL))
                != SSL_SUCCESS)
        {
            msg(M_FATAL,
                    "wolfSSL_CTX_load_verify_locations failed with Errno: %d",
                    ret);
            return;
        }
    }
}

int
tls_ctx_load_priv_file(struct tls_root_ctx *ctx, const char *priv_key_file,
                       bool priv_key_inline)
{

    int ret;
    int key_len;
    ASSERT(ctx != NULL);

    if (priv_key_inline)
    {
        /* Key in memory */
        if ((key_len = strlen(priv_key_file)) == 0)
        {
            msg(M_FATAL, "Empty certificate passed.");
            return 1;
        }
        if ((ret = wolfSSL_CTX_use_PrivateKey_buffer(ctx->ctx,
                (uint8_t*) priv_key_file, key_len,
                SSL_FILETYPE_PEM)) != SSL_SUCCESS)
        {
            msg(M_FATAL,
                    "wolfSSL_CTX_use_PrivateKey_buffer failed with Errno: %d",
                    ret);
            return 1;
        }
    }
    else
    {
        /* Key in file */
        if ((ret = wolfSSL_CTX_use_PrivateKey_file(ctx->ctx, priv_key_file,
        SSL_FILETYPE_PEM)) != SSL_SUCCESS)
        {
            msg(M_FATAL,
                    "wolfSSL_CTX_use_PrivateKey_file failed with Errno: %d",
                    ret);
            return 1;
        }
    }
    return 0;
}

#ifdef ENABLE_MANAGEMENT
int tls_ctx_use_management_external_key(struct tls_root_ctx *ctx)
{
    msg(M_INFO, "%s: key already loaded", __func__);
    return 1;
}
#endif /* ENABLE_MANAGEMENT */

void
tls_ctx_load_ca(struct tls_root_ctx *ctx, const char *ca_file,
                bool ca_inline, const char *ca_path, bool tls_server)
{
    int ca_len, ret;

    ASSERT(ctx != NULL);

    if (ca_inline)
    {
        /* Certificate in memory */
        if ((ca_len = strlen(ca_file)) == 0)
        {
            msg(M_FATAL, "Empty certificate passed.");
        }

        if ((ret = wolfSSL_CTX_load_verify_buffer(ctx->ctx,
                (uint8_t*) ca_file, ca_len,
                SSL_FILETYPE_PEM)) != SSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_load_verify_buffer failed with Errno: %d",
                    ret);
        }
        if (ca_path)
        {
            if ((ret = wolfSSL_CTX_load_verify_locations(ctx->ctx, NULL,
                    ca_path)) != SSL_SUCCESS)
            {
                msg(M_FATAL,
                        "wolfSSL_CTX_load_verify_locations failed with Errno: %d",
                        ret);
            }
        }
    }
    else
    {
        /* Certificate in file */
        if ((ret = wolfSSL_CTX_load_verify_locations(ctx->ctx, ca_file, ca_path))
                != SSL_SUCCESS)
        {
            msg(M_FATAL,
                    "wolfSSL_CTX_load_verify_locations failed with Errno: %d",
                    ret);
        }
    }
}

void
tls_ctx_load_extra_certs(struct tls_root_ctx *ctx, const char *extra_certs_file,
                         bool extra_certs_inline)
{
    tls_ctx_load_ca(ctx, extra_certs_file, extra_certs_inline, NULL,
    false);
}

void
backend_tls_ctx_reload_crl(struct tls_root_ctx *ssl_ctx, const char *crl_file,
                           bool crl_inline)
{
    int ret, len;
    if (crl_inline)
    {
        /* CRL in memory */
        if ((len = strlen(crl_file)) == 0)
        {
            msg(M_FATAL, "Empty CRL passed.");
        }
        if ((ret = wolfSSL_CTX_LoadCRLBuffer(ssl_ctx->ctx,
                (unsigned char*) crl_file, len, SSL_FILETYPE_PEM))
                != SSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_LoadCRLBuffer failed with Errno: %d",
                    ret);
        }
    }
    else
    {
        /* CRL in file */
        if ((ret = wolfSSL_CTX_LoadCRL(ssl_ctx->ctx, crl_file, SSL_FILETYPE_PEM,
                0)) != SSL_SUCCESS)
        {
            msg(M_FATAL, "wolfSSL_CTX_LoadCRL failed with Errno: %d", ret);
        }
    }
}

/* **************************************
 *
 * Key-state specific functions
 *
 * **************************************/

/*
 * SSL is handled by library (wolfSSL in this case) but data is dumped
 * to buffers instead of being sent directly through TCP sockets. OpenVPN
 * itself handles sending and receiving data.
 */

static int ssl_buff_read(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    struct list_buffer_t* ssl_buf = (struct list_buffer_t*) ctx;
    struct bucket_t* b;
    uint32_t l, ret = 0, len = sz;

    if (!ssl_buf->first || !ssl_buf->len)
    {
        return WOLFSSL_CBIO_ERR_WANT_READ;
    }

    while (len && ssl_buf->len)
    {
        l = MIN(len, ssl_buf->first->len);
        memcpy(buf, ssl_buf->first->buf + ssl_buf->first->offset, l);
        ssl_buf->first->offset += l;
        ssl_buf->first->len -= l;
        ssl_buf->len -= l;
        len -= l;
        buf += l;
        ret += l;
        if (!ssl_buf->first->len)
        {
            /* Bucket is wholly read */
            if (ssl_buf->first != ssl_buf->last)
            {
                /* Free bucket and go to next one */
                b = ssl_buf->first;
                ssl_buf->first = ssl_buf->first->next;
                free(b);
            }
            else
            {
                /*
                 * Reset and keep one bucket so that we don't have to
                 * malloc buckets for many small messages.
                 */
                ssl_buf->first->len = 0;
                ssl_buf->first->offset = 0;
            }
        }
    }

    return ret;
}

static void allocate_new_bucket(struct bucket_t** last,
        struct bucket_t** second_last, uint32_t* len, char** buf)
{
    uint32_t l;

    *last = *second_last = (struct bucket_t*) malloc(sizeof(struct bucket_t));
    check_malloc_return(*last);

    l = MIN(*len, BUCKET_BUF_LEN);
    (*last)->len = l;
    (*last)->offset = 0;
    (*last)->next = NULL;
    memcpy((*last)->buf, *buf, l);

    *len -= l;
    *buf += l;
}

static int ssl_buff_write(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    struct list_buffer_t* ssl_buf = (struct list_buffer_t*) ctx;
    uint32_t l, len = sz;

    if (len == 0)
    {
        return 0;
    }

    ssl_buf->len += len;

    if (!ssl_buf->first)
    {
        /* First time ssl_buff_write so we need to allocate the first bucket. */

        allocate_new_bucket(&ssl_buf->last, &ssl_buf->first, &len, &buf);
    }

    while (len)
    {
        l = MIN(len,
                BUCKET_BUF_LEN - (ssl_buf->last->offset + ssl_buf->last->len));
        if (l)
        {
            /* If there is any room in the last bucket then copy */
            memcpy(
                    ssl_buf->last->buf + ssl_buf->last->offset
                            + ssl_buf->last->len, buf, l);
            len -= l;
            buf += l;
            ssl_buf->last->len += l;
        }

        if (!len)
        {
            /* No more data to write */
            break;
        }

        allocate_new_bucket(&ssl_buf->last, &ssl_buf->last->next, &len, &buf);
    }

    return sz;
}

void key_state_ssl_init(struct key_state_ssl *ks_ssl,
        const struct tls_root_ctx *ssl_ctx, bool is_server,
        struct tls_session *session)
{
    int err;

    ASSERT(ssl_ctx != NULL);

    if ((ks_ssl->ssl = wolfSSL_new(ssl_ctx->ctx)) == NULL)
    {
        msg(M_FATAL, "wolfSSL_new failed");
    }

    if ((ks_ssl->send_buf = (struct list_buffer_t*) calloc(
            sizeof(struct list_buffer_t), 1)) == NULL)
    {
        wolfSSL_free(ks_ssl->ssl);
        msg(M_FATAL, "Failed to allocate memory for send buffer.");
    }

    if ((ks_ssl->recv_buf = (struct list_buffer_t*) calloc(
            sizeof(struct list_buffer_t), 1)) == NULL)
    {
        free(ks_ssl->send_buf);
        wolfSSL_free(ks_ssl->ssl);
        msg(M_FATAL, "Failed to allocate memory for receive buffer.");
    }

    /* Register functions handling queueing of data in buffers */
    wolfSSL_SSLSetIORecv(ks_ssl->ssl, &ssl_buff_read);
    wolfSSL_SSLSetIOSend(ks_ssl->ssl, &ssl_buff_write);

    /* Register pointers to appropriate buffers */
    wolfSSL_SetIOWriteCtx(ks_ssl->ssl, ks_ssl->send_buf);
    wolfSSL_SetIOReadCtx(ks_ssl->ssl, ks_ssl->recv_buf);

    if (is_server)
    {
        if ((err = wolfSSL_accept(ks_ssl->ssl)) != SSL_SUCCESS)
        {
            err = wolfSSL_get_error(ks_ssl->ssl, err);
            switch (err)
            {
            case WOLFSSL_ERROR_WANT_WRITE:
            case WOLFSSL_ERROR_WANT_READ:
                break;
            default:
                msg(M_FATAL, "wolfSSL_accept failed");
            }
        }
    }
    else
    {
        if ((err = wolfSSL_connect(ks_ssl->ssl)) != SSL_SUCCESS)
        {
            err = wolfSSL_get_error(ks_ssl->ssl, err);
            switch (err)
            {
            case WOLFSSL_ERROR_WANT_WRITE:
            case WOLFSSL_ERROR_WANT_READ:
                break;
            default:
                msg(M_FATAL, "wolfSSL_connect failed");
            }
        }
    }

    ks_ssl->session = session;
    wolfSSL_SetCertCbCtx(ks_ssl->ssl, ks_ssl);
}

void key_state_ssl_free(struct key_state_ssl *ks_ssl)
{
    struct bucket_t* b;
    struct bucket_t* c;
    wolfSSL_free(ks_ssl->ssl);
    if (ks_ssl->recv_buf)
    {
        b = ks_ssl->recv_buf->first;
        while (b)
        {
            c = b->next;
            free(b);
            b = c;
        }
        free(ks_ssl->recv_buf);
    }
    if (ks_ssl->send_buf)
    {
        b = ks_ssl->send_buf->first;
        while (b)
        {
            c = b->next;
            free(b);
            b = c;
        }
        free(ks_ssl->send_buf);
    }
    ks_ssl->ssl = NULL;
    ks_ssl->recv_buf = NULL;
    ks_ssl->send_buf = NULL;
    ks_ssl->session = NULL;
}

void key_state_export_keying_material(struct key_state_ssl *ks_ssl,
        struct tls_session *session)
{
    msg(M_FATAL, "%s not supported by wolfSSL", __func__);
}

int key_state_write_plaintext(struct key_state_ssl *ks_ssl, struct buffer *buf)
{
    int ret = 1;
    perf_push(PERF_BIO_WRITE_PLAINTEXT);

    ASSERT(ks_ssl != NULL);

    switch (key_state_write_plaintext_const(ks_ssl, BPTR(buf), BLEN(buf)))
    {
    case 1:
        ret = 1;
        memset(BPTR(buf), 0, BLEN(buf)); /* erase data just written */
        buf->len = 0;
        break;
    case 0:
        ret = 0;
        break;
    case -1:
        ret = -1;
        break;
    default:
        msg(M_WARN, "Invalid error code from key_state_write_plaintext_const");
        break;
    }

    perf_pop();
    return ret;
}

int key_state_write_plaintext_const(struct key_state_ssl *ks_ssl,
        const uint8_t *data, int len)
{
    int err = 0;
    int ret = 1;
    perf_push(PERF_BIO_WRITE_PLAINTEXT);

    ASSERT(ks_ssl != NULL);

    if (len > 0)
    {
        if ((err = wolfSSL_write(ks_ssl->ssl, data, len)) != len)
        {
            err = wolfSSL_get_error(ks_ssl->ssl, err);
            switch (err)
            {
            case WOLFSSL_ERROR_WANT_WRITE:
            case WOLFSSL_ERROR_WANT_READ:
                ret = 0;
                break;
            default:
                msg(M_WARN, "wolfSSL_write failed with Error: %s",
                        wc_GetErrorString(err));
                ret = -1;
                break;
            }
        }
    }

    perf_pop();
    return ret;
}

int key_state_read_ciphertext(struct key_state_ssl *ks_ssl, struct buffer *buf,
        int maxlen)
{
    int ret = 1;
    perf_push(PERF_BIO_READ_CIPHERTEXT);

    if (BLEN(buf) != 0)
    {
        ret = 0;
        goto cleanup;
    }

    ASSERT(ks_ssl != NULL);
    buf->len = ssl_buff_read(ks_ssl->ssl, (char*) BPTR(buf), maxlen,
            ks_ssl->send_buf);

    ret = buf->len > 0 ? 1 : 0;

    cleanup: perf_pop();
    return ret;
}

int key_state_write_ciphertext(struct key_state_ssl *ks_ssl, struct buffer *buf)
{
    int err, ret = 1;
    perf_push(PERF_BIO_WRITE_CIPHERTEXT);

    ASSERT(ks_ssl != NULL);

    if (BLEN(buf) > 0)
    {
        if ((err = (ssl_buff_write(ks_ssl->ssl, (char*) BPTR(buf), BLEN(buf),
                ks_ssl->recv_buf))) != BLEN(buf))
        {
            ret = 0;
            goto cleanup;
        }
        memset(BPTR(buf), 0, BLEN(buf)); /* erase data just written */
        buf->len = 0;
    }

    cleanup: perf_pop();
    return ret;
}

int key_state_read_plaintext(struct key_state_ssl *ks_ssl, struct buffer *buf,
        int maxlen)
{
    int err, ret = 1;
    perf_push(PERF_BIO_READ_PLAINTEXT);

    ASSERT(ks_ssl != NULL);

    if (BLEN(buf) != 0)
    {
        ret = 0;
        goto cleanup;
    }

    if ((err = wolfSSL_read(ks_ssl->ssl, BPTR(buf), maxlen)) < 0)
    {
        err = wolfSSL_get_error(ks_ssl->ssl, err);
        switch (err)
        {
        case WOLFSSL_ERROR_WANT_WRITE:
        case WOLFSSL_ERROR_WANT_READ:
            ret = 0;
            goto cleanup;
        default:
            msg(M_WARN, "wolfSSL_read failed with Error: %s",
                    wc_GetErrorString(err));
            ret = -1;
            goto cleanup;
        }
    }
    buf->len = err;

    cleanup: perf_pop();
    return ret;
}

void print_details(struct key_state_ssl *ks_ssl, const char *prefix)
{
    WOLFSSL_X509 *cert = NULL;
    const WOLFSSL_CIPHER *ciph;
    WOLFSSL_EVP_PKEY *key = NULL;
    char s1[256];
    char s2[256];
    char desc[256];
    const WOLFSSL_EC_GROUP* group;

    openvpn_snprintf(s2, sizeof(s2), "%s %s", prefix,
            wolfSSL_get_version(ks_ssl->ssl));

    ciph = wolfSSL_get_current_cipher(ks_ssl->ssl);
    if (wolfSSL_CIPHER_description(ciph, desc, 256))
    {
        openvpn_snprintf(s1, sizeof(s1), "%s, cipher %s", s2, desc);
    }

    if ((cert = wolfSSL_get_peer_certificate(ks_ssl->ssl)) && (key =
            wolfSSL_X509_get_pubkey(cert)))
    {
        memcpy(s2, s1, sizeof(s1));

        switch (wolfSSL_X509_get_pubkey_type(cert))
        {
        case RSAk:
            openvpn_snprintf(s1, sizeof(s1), "%s, %d bit RSA", s2,
                    wolfSSL_EVP_PKEY_bits(key));
            break;
        default:
            /*
             * wolfSSL only supports RSA and ECC certificate public keys so if it isn't RSA then
             * it must be ECC.
             */
            if ((group = wolfSSL_EC_KEY_get0_group(key->ecc))
                    && (wc_ecc_is_valid_idx(group->curve_idx)))
            {
                openvpn_snprintf(s1, sizeof(s1), "%s, %d bit EC, curve: %s", s2,
                        wolfSSL_EVP_PKEY_bits(key),
                        wc_ecc_get_name(wc_ecc_get_curve_id(group->curve_idx)));
            }
            else
            {
                openvpn_snprintf(s1, sizeof(s1),
                        "%s, %d bit EC, curve: Error getting curve name", s2,
                        wolfSSL_EVP_PKEY_bits(key));
            }
            break;
        }
    }

    msg(D_HANDSHAKE, "%s", s1);

    if (key)
    {
        wolfSSL_EVP_PKEY_free(key);
    }
    if (cert)
    {
        wolfSSL_X509_free(cert);
    }
}

void show_available_tls_ciphers_list(const char *cipher_list,
        const char *tls_cert_profile,
        bool tls13)
{
    int i;
    char* cipher;
    WOLFSSL *ssl;
    WOLFSSL_CTX *ctx;

#ifdef WOLFSSL_TLS13
    if (tls13)
    {
        if ((ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method())) == NULL)
        {
            wolfSSL_CTX_free(ctx);
            msg(M_FATAL, "wolfSSL_CTX_new failed");
        }
    }
    else
#else
        msg(M_FATAL, "wolfSSL library compiled without TLS 1.3 support.");
#endif
    {
        if ((ctx = wolfSSL_CTX_new(wolfSSLv23_client_method())) == NULL)
        {
            wolfSSL_CTX_free(ctx);
            msg(M_FATAL, "wolfSSL_CTX_new failed");
        }
    }
    if (cipher_list)
    {
        if (wolfSSL_CTX_set_cipher_list(ctx, cipher_list) != SSL_SUCCESS)
        {
            msg(M_FATAL, "Failed to set ciphers: %s", cipher_list);
        }
    }
    if ((ssl = wolfSSL_new(ctx)) == NULL)
    {
        msg(M_FATAL, "wolfSSL_new failed");
    }
    for (i = 0; (cipher = wolfSSL_get_cipher_list_ex(ssl, i)); i++)
    {
        if (tls13 && !strncmp(cipher, "TLS13-", 6))
        {
            printf("%s\n", cipher);
        }
        else if (!tls13 && strncmp(cipher, "TLS13-", 6))
        {
            printf("%s\n", cipher);
        }
    }
}

void show_available_curves(void)
{
#ifdef HAVE_ECC
    int i;
    printf("Available Elliptic curves:\n");
    for (i = 0; wc_ecc_is_valid_idx(i); i++)
    {
        printf("%s\n", wc_ecc_get_name(wc_ecc_get_curve_id(i)));
    }
#else
    msg(M_FATAL, "wolfSSL library compiled without ECC support.");
#endif
}

void get_highest_preference_tls_cipher(char *buf, int size)
{
    WOLFSSL *ssl;
    WOLFSSL_CTX* ctx;
    const char* cipher_name;

    if ((ctx = wolfSSL_CTX_new(wolfSSLv23_client_method())) == NULL)
    {
        wolfSSL_CTX_free(ctx);
        msg(M_FATAL, "wolfSSL_CTX_new failed");
    }

    if ((ssl = wolfSSL_new(ctx)) == NULL)
    {
        msg(M_FATAL, "wolfSSL_new failed");
    }

    cipher_name = wolfSSL_get_cipher_name(ssl);
    if (cipher_name)
    {
        strncpynt(buf, cipher_name, size);
    }
    else
    {
        msg(M_WARN, "wolfSSL_get_cipher_name failed");
    }

    wolfSSL_free(ssl);
    wolfSSL_CTX_free(ctx);
}

const char * get_ssl_library_version(void)
{
    return wolfSSL_lib_version();
}

#endif /* ENABLE_CRYPTO_WOLFSSL */
