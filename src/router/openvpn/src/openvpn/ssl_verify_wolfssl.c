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
 * @file Control Channel Verification Module wolfSSL backend
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#include "syshead.h"

#if defined(ENABLE_CRYPTO_WOLFSSL)

#include "ssl_backend.h"
#include "ssl_verify.h"
#include "ssl_verify_backend.h"

int verify_callback(int preverify_ok, WOLFSSL_X509_STORE_CTX *store)
{
    char buffer[WOLFSSL_MAX_ERROR_SZ];
    struct key_state_ssl* ks_ssl = store->userCtx;

    if (store->error)
    {
        msg(M_INFO, "In verification callback, error = %d, %s\n", store->error,
                wolfSSL_ERR_error_string(store->error, buffer));
        return 0;
    }
    else
    {
        if (verify_cert(ks_ssl->session, store->current_cert,
                store->error_depth) != SUCCESS)
        {
            return 0;
        }
        return 1;
    }
}

char *x509_get_subject(openvpn_x509_cert_t *cert, struct gc_arena *gc)
{
    char *subject = NULL;
    int subject_len;
    WOLFSSL_X509_NAME* name = wolfSSL_X509_get_subject_name(cert);
    if (!name)
    {
        return NULL;
    }
    subject_len = wolfSSL_X509_NAME_get_sz(name);

    subject = gc_malloc(subject_len, FALSE, gc);
    check_malloc_return(subject);

    return wolfSSL_X509_NAME_oneline(name, subject, subject_len);
}

struct buffer x509_get_sha1_fingerprint(openvpn_x509_cert_t *cert,
        struct gc_arena *gc)
{
    unsigned int hashSz = wc_HashGetDigestSize(WC_HASH_TYPE_SHA);
    struct buffer hash = alloc_buf_gc(hashSz, gc);
    check_malloc_return(BPTR(&hash));
    if (wolfSSL_X509_digest(cert, wolfSSL_EVP_sha1(), BPTR(&hash),
            &hashSz) != SSL_SUCCESS)
    {
        msg(M_FATAL, "wolfSSL_X509_digest for sha1 failed.");
    }
    return hash;
}

struct buffer x509_get_sha256_fingerprint(openvpn_x509_cert_t *cert,
        struct gc_arena *gc)
{
    unsigned int hashSz = wc_HashGetDigestSize(WC_HASH_TYPE_SHA256);
    struct buffer hash = alloc_buf_gc(hashSz, gc);
    check_malloc_return(BPTR(&hash));
    if (wolfSSL_X509_digest(cert, wolfSSL_EVP_sha256(), BPTR(&hash),
            &hashSz) != SSL_SUCCESS)
    {
        msg(M_FATAL, "wolfSSL_X509_digest for sha256 failed.");
    }
    return hash;
}

#ifdef ENABLE_X509ALTUSERNAME
static enum Extensions_Sum x509_username_field_ext_get_nid(const char *extname)
{
    if (!strcmp(extname, "subjectAltName"))
    {
        return ALT_NAMES_OID;
    }
    else if (!strcmp(extname, "issuerAltName"))
    {
        return ISSUE_ALT_NAMES_OID;
    }
    else
    {
        return 0;
    }
}
bool x509_username_field_ext_supported(const char *extname)
{
    return x509_username_field_ext_get_nid(extname) != 0;
}
#endif

result_t backend_x509_get_username(char *common_name, int cn_len,
        char *x509_username_field, openvpn_x509_cert_t *peer_cert)
{
    result_t res = FAILURE;

    char *subject = NULL;
#ifdef ENABLE_X509ALTUSERNAME
    WOLFSSL_STACK *ext = NULL;
    bool found = false;
    if (strncmp("ext:", x509_username_field, 4) == 0)
    {
        int i;
        int nid;
        char szOid[1024];
        WOLFSSL_ASN1_OBJECT *name;

        if (!(nid = x509_username_field_ext_get_nid(x509_username_field + 4)))
        {
            msg(D_TLS_ERRORS,
                    "ERROR: --x509-username-field 'ext:%s' not supported",
                    x509_username_field + 4);
            goto failure;
        }

        if (!(ext = wolfSSL_X509_get_ext_d2i(peer_cert, nid, NULL, NULL)))
        {
            goto failure;
        }

        for (i = 0; i < wolfSSL_sk_GENERAL_NAME_num(ext); i++)
        {
            name = wolfSSL_sk_GENERAL_NAME_value(ext, i);
            if (name->type == ASN_RFC822_TYPE)
            { /* Check if email type */
                if (wolfSSL_OBJ_obj2txt(szOid, sizeof(szOid), name, 0) > 0)
                {
                    strncpy(common_name, szOid, cn_len);
                    found = true;
                    break;
                }
                else if (name->obj)
                {
                    strncpy(common_name, (char*) name->obj, cn_len);
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            goto failure;
        }
    }
    else
#endif
    {
        char *c;
        char *start_pos;
        int field_len = strlen(x509_username_field);
        int value_len;
        WOLFSSL_X509_NAME* name = wolfSSL_X509_get_subject_name(peer_cert);
        if (!name)
        {
            goto failure;
        }
        subject = wolfSSL_X509_NAME_oneline(name, NULL, 0);

        for (c = subject; *c != '\0'; c++)
        {
            if (*c == '/'
                    && strncmp(c + 1, x509_username_field, field_len) == 0)
            {
                c += field_len + 1; // increment to value of field
                start_pos = c + 1;
                while (*(++c) != '/' && *c != '\0')
                    ; // inc to next slash or end of string
                value_len = MIN(c - start_pos, cn_len - 1);
                memcpy(common_name, start_pos, value_len);
                common_name[value_len] = '\0';
                break;
            }
        }
    }

    res = SUCCESS;
    failure: if (subject)
    {
        free(subject);
    }
#ifdef ENABLE_X509ALTUSERNAME
    if (ext)
    {
        wolfSSL_sk_ASN1_OBJECT_free(ext);
    }
#endif
    return res;
}

char *backend_x509_get_serial(openvpn_x509_cert_t *cert, struct gc_arena *gc)
{
    uint8_t buf[EXTERNAL_SERIAL_SIZE];
    int buf_len = EXTERNAL_SERIAL_SIZE, ret, radix_size;
    mp_int big_num;
    struct buffer dec_string;

    /*
     * The serial number buffer is in big endian.
     */
    if ((ret = wolfSSL_X509_get_serial_number(cert, buf, &buf_len))
            != WOLFSSL_SUCCESS)
    {
        msg(M_FATAL, "wolfSSL_X509_get_serial_number failed with Errno: %d",
                ret);
    }

    if (mp_init(&big_num) != MP_OKAY)
    {
        msg(M_FATAL, "mp_init failed");
    }

    if ((ret = mp_read_unsigned_bin(&big_num, buf, buf_len)) != MP_OKAY)
    {
        msg(M_FATAL, "mp_read_unsigned_bin failed with Errno: %d", ret);
    }

    if ((ret = mp_radix_size(&big_num, MP_RADIX_DEC, &radix_size)) != MP_OKAY)
    {
        msg(M_FATAL, "mp_radix_size failed with Errno: %d", ret);
    }

    dec_string = alloc_buf_gc(radix_size, gc);
    check_malloc_return(BPTR(&dec_string));

    if ((ret = mp_todecimal(&big_num, (char*) BPTR(&dec_string))) != MP_OKAY)
    {
        msg(M_FATAL, "mp_todecimal failed with Errno: %d", ret);
    }

    dec_string.len = radix_size;

    return (char*) BPTR(&dec_string);
}

char *backend_x509_get_serial_hex(openvpn_x509_cert_t *cert,
        struct gc_arena *gc)
{
    uint8_t buf[EXTERNAL_SERIAL_SIZE];
    int buf_len = EXTERNAL_SERIAL_SIZE, ret;

    /*
     * The serial number buffer is in big endian.
     */
    if ((ret = wolfSSL_X509_get_serial_number(cert, buf, &buf_len))
            != WOLFSSL_SUCCESS)
    {
        msg(M_FATAL, "wolfSSL_X509_get_serial_number failed with Errno: %d",
                ret);
    }

    return format_hex_ex(buf, buf_len, 0, 1, ":", gc);
}

void x509_setenv(struct env_set *es, int cert_depth, openvpn_x509_cert_t *cert)
{
    char *subject;
    char *c;
    char *name_start_pos;
    int name_len;
    char *value_start_pos;
    int value_len;
    char* name_buf = NULL;
    char* value_buf = NULL;
    char *full_name_buf = NULL;
    int full_name_len;
    WOLFSSL_X509_NAME* name = wolfSSL_X509_get_subject_name(cert);
    if (!name)
    {
        return;
    }
    subject = wolfSSL_X509_NAME_oneline(name, NULL, 0);

    for (c = subject; *c != '\0';)
    {
        ASSERT(*c == '/'); // c should point to slash on each loop

        name_start_pos = c + 1;
        while (*(++c) != '=' && *c != '\0')
            ; // increment to equals sign
        name_len = c - name_start_pos;

        value_start_pos = c + 1;
        while (*(++c) != '/' && *c != '\0')
            ; // increment to next slash
        value_len = c - value_start_pos;

        /*
         * length of buffer is: length of name + null teminator +
         *                      6 chars from naming convention +
         *                      5 chars for depth number (should be enough)
         */
        full_name_len = name_len + 1 + 6 + 5;

        name_buf = realloc(name_buf, name_len + 1);
        check_malloc_return(name_buf);
        full_name_buf = realloc(full_name_buf, full_name_len);
        check_malloc_return(full_name_buf);
        value_buf = realloc(value_buf, value_len + 1);
        check_malloc_return(value_buf);

        memcpy(name_buf, name_start_pos, name_len);
        memcpy(value_buf, value_start_pos, value_len);
        name_buf[name_len] = '\0';
        value_buf[value_len] = '\0';

        openvpn_snprintf(full_name_buf, full_name_len, "X509_%d_%s", cert_depth,
                name_buf);

        setenv_str_incr(es, full_name_buf, value_buf);
    }

    if (name_buf)
    {
        free(name_buf);
    }
    if (full_name_buf)
    {
        free(full_name_buf);
    }
    if (value_buf)
    {
        free(value_buf);
    }
    free(subject);
}

void x509_track_add(const struct x509_track **ll_head, const char *name,
        int msglevel, struct gc_arena *gc)
{
    struct x509_track *xt;
    ALLOC_OBJ_CLEAR_GC(xt, struct x509_track, gc);
    if (*name == '+')
    {
        xt->flags |= XT_FULL_CHAIN;
        ++name;
    }
    xt->name = name;
    xt->nid = wolfSSL_OBJ_txt2nid(name);
    if (xt->nid != NID_undef)
    {
        xt->next = *ll_head;
        *ll_head = xt;
    }
    else
    {
        msg(msglevel, "x509_track: no such attribute '%s'", name);
    }
}

/* worker method for setenv_x509_track */
static void do_setenv_x509(struct env_set *es, const char *name, char *value,
        int depth)
{
    char *name_expand;
    size_t name_expand_size;

    string_mod(value, CC_ANY, CC_CRLF, '?');
    msg(D_X509_ATTR, "X509 ATTRIBUTE name='%s' value='%s' depth=%d", name,
            value, depth);
    name_expand_size = 64 + strlen(name);
    name_expand = (char *) malloc(name_expand_size);
    check_malloc_return(name_expand);
    openvpn_snprintf(name_expand, name_expand_size, "X509_%d_%s", depth, name);
    setenv_str(es, name_expand, value);
    free(name_expand);
}

void x509_setenv_track(const struct x509_track *xt, struct env_set *es,
        const int depth, openvpn_x509_cert_t *x509)
{
    struct gc_arena gc = gc_new();
    struct buffer fp_buf;
    char *fp_str = NULL;
    int i;
    WOLFSSL_X509_NAME *x509_name = wolfSSL_X509_get_subject_name(x509);
    while (xt)
    {
        if (depth == 0 || (xt->flags & XT_FULL_CHAIN))
        {
            switch (xt->nid)
            {
            case NID_sha1:
            case NID_sha256:
                if (xt->nid == NID_sha1)
                {
                    fp_buf = x509_get_sha1_fingerprint(x509, &gc);
                }
                else
                {
                    fp_buf = x509_get_sha256_fingerprint(x509, &gc);
                }
                fp_str = format_hex_ex(BPTR(&fp_buf), BLEN(&fp_buf), 0,
                        1 | FHE_CAPS, ":", &gc);
                do_setenv_x509(es, xt->name, fp_str, depth);
                break;
            default:
                i = wolfSSL_X509_NAME_get_index_by_NID(x509_name, xt->nid, -1);
                if (i >= 0)
                {
                    WOLFSSL_X509_NAME_ENTRY *ent = wolfSSL_X509_NAME_get_entry(
                            x509_name, i);
                    if (ent)
                    {
                        WOLFSSL_ASN1_STRING *val =
                                wolfSSL_X509_NAME_ENTRY_get_data(ent);
                        do_setenv_x509(es, xt->name, val->data, depth);
                    }
                }
                else
                {
                    WOLFSSL_STACK *ext = wolfSSL_X509_get_ext_d2i(x509, xt->nid,
                    NULL, NULL);
                    if (ext)
                    {
                        for (i = 0; i < wolfSSL_sk_GENERAL_NAME_num(ext); i++)
                        {
                            WOLFSSL_ASN1_OBJECT *oid =
                                    wolfSSL_sk_GENERAL_NAME_value(ext, i);
                            char szOid[1024];

                            if (wolfSSL_OBJ_obj2txt(szOid, sizeof(szOid), oid,
                                    0) > 0)
                            {
                                do_setenv_x509(es, xt->name, szOid, depth);
                                break;
                            }
                            else if (wolfSSL_OBJ_obj2txt(szOid, sizeof(szOid),
                                    oid, 1) > 0)
                            {
                                do_setenv_x509(es, xt->name, szOid, depth);
                                break;
                            }
                        }
                    }
                    wolfSSL_sk_ASN1_OBJECT_free(ext);
                }
            }
        }
        xt = xt->next;
    }
    gc_free(&gc);
}

result_t x509_verify_ns_cert_type(openvpn_x509_cert_t *cert, const int usage)
{
    if (usage == NS_CERT_CHECK_NONE)
    {
        return SUCCESS;
    }

    msg(M_FATAL,
            "wolfSSL does not grant access to the Netscape Cert Type extension.");
    return FAILURE;
}

result_t x509_verify_cert_ku(openvpn_x509_cert_t *x509,
        const unsigned * const expected_ku, int expected_len)
{
    unsigned int ku = wolfSSL_X509_get_keyUsage(x509);

    if (ku == 0)
    {
        msg(D_TLS_ERRORS, "Certificate does not have key usage extension");
        return FAILURE;
    }

    if (expected_ku[0] == OPENVPN_KU_REQUIRED)
    {
        /* Extension required, value checked by TLS library */
        return SUCCESS;
    }

    /*
     * Fixup if no LSB bits
     */
    if ((ku & 0xff) == 0)
    {
        ku >>= 8;
    }

    msg(D_HANDSHAKE, "Validating certificate key usage");
    result_t found = FAILURE;
    for (size_t i = 0; i < expected_len; i++)
    {
        if (expected_ku[i] != 0 && (ku & expected_ku[i]) == expected_ku[i])
        {
            found = SUCCESS;
            break;
        }
    }

    if (found != SUCCESS)
    {
        msg(D_TLS_ERRORS,
                "ERROR: Certificate has key usage %04x, expected one of:", ku);
        for (size_t i = 0; i < expected_len && expected_ku[i]; i++)
        {
            msg(D_TLS_ERRORS, " * %04x", expected_ku[i]);
        }
    }

    return found;
}

static const char* oid_translate_num_to_str(const char* oid)
{
    static const struct oid_dict
    {
        char* num;
        char* desc;
    } oid_dict[] =
    {
    { "2.5.29.37.0", "Any Extended Key Usage" },
    { "1.3.6.1.5.5.7.3.1", "TLS Web Server Authentication" },
    { "1.3.6.1.5.5.7.3.2", "TLS Web Client Authentication" },
    { "1.3.6.1.5.5.7.3.3", "Code Signing" },
    { "1.3.6.1.5.5.7.3.4", "E-mail Protection" },
    { "1.3.6.1.5.5.7.3.8", "Time Stamping" },
    { "1.3.6.1.5.5.7.3.9", "OCSP Signing" },
    { NULL, NULL } };
    const struct oid_dict* idx;
    for (idx = oid_dict; idx->num != NULL; idx++)
    {
        if (!strcmp(oid, idx->num))
        {
            return idx->desc;
        }
    }
    return NULL;
}

result_t x509_verify_cert_eku(openvpn_x509_cert_t *x509,
        const char * const expected_oid)
{
    WOLFSSL_STACK *eku = NULL;
    result_t found = FAILURE;
    const char* desc;

    if ((eku = (WOLFSSL_STACK *) wolfSSL_X509_get_ext_d2i(x509,
            EXT_KEY_USAGE_OID,
            NULL, NULL)) == NULL)
    {
        msg(D_HANDSHAKE,
                "Certificate does not have extended key usage extension");
    }
    else
    {
        int i;
        msg(D_HANDSHAKE, "Validating certificate extended key usage");
        for (i = 0; i < wolfSSL_sk_GENERAL_NAME_num(eku); i++)
        {
            WOLFSSL_ASN1_OBJECT *oid = wolfSSL_sk_GENERAL_NAME_value(eku, i);
            char szOid[1024];

            if (wolfSSL_OBJ_obj2txt(szOid, sizeof(szOid), oid, 0) > 0)
            {
                msg(D_HANDSHAKE, "++ Certificate has EKU (str) %s, expects %s",
                        szOid, expected_oid);
                if (!strcmp(expected_oid, szOid))
                {
                    found = SUCCESS;
                    break;
                }
            }
            if (wolfSSL_OBJ_obj2txt(szOid, sizeof(szOid), oid, 1) > 0)
            {
                msg(D_HANDSHAKE, "++ Certificate has EKU (oid) %s, expects %s",
                        szOid, expected_oid);
                if (!strcmp(expected_oid, szOid))
                {
                    found = SUCCESS;
                    break;
                }
                if ((desc = oid_translate_num_to_str(szOid)) != NULL)
                {
                    msg(D_HANDSHAKE, "++ oid %s translated to %s", szOid, desc);
                    if (!strcmp(expected_oid, desc))
                    {
                        found = SUCCESS;
                        break;
                    }
                }
            }
        }
    }

    if (eku != NULL)
    {
        wolfSSL_sk_GENERAL_NAME_pop_free(eku, NULL);
    }

    return found;
}

result_t x509_write_pem(FILE *peercert_file, openvpn_x509_cert_t *peercert)
{
    if (wolfSSL_PEM_write_X509(peercert_file, peercert) < 0)
    {
        msg(M_NONFATAL, "Failed to write peer certificate in PEM format");
        return FAILURE;
    }
    return SUCCESS;
}

bool tls_verify_crl_missing(const struct tls_options *opt)
{
    /*
     * This is checked at load time.
     */
    return false;
}

#endif /* ENABLE_CRYPTO_WOLFSSL */
