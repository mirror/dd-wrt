/*
 * snmp_openssl.c
 *
 * Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/net-snmp-features.h>

/** OpenSSL compat functions for apps */
#if defined(NETSNMP_USE_OPENSSL)

#include <string.h>
#include <openssl/dh.h>

#ifndef HAVE_DH_GET0_PQG
void
DH_get0_pqg(const DH *dh, const BIGNUM **p, const BIGNUM **q, const BIGNUM **g)
{
   if (p != NULL)
       *p = dh->p;
   if (q != NULL)
       *q = dh->q;
   if (g != NULL)
       *g = dh->g;
}
#endif

#ifndef HAVE_DH_GET0_KEY
void
DH_get0_key(const DH *dh, const BIGNUM **pub_key, const BIGNUM **priv_key)
{
   if (pub_key != NULL)
       *pub_key = dh->pub_key;
   if (priv_key != NULL)
       *priv_key = dh->priv_key;
}
#endif

#ifndef HAVE_DH_SET0_PQG
int
DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
   /* If the fields p and g in d are NULL, the corresponding input
    * parameters MUST be non-NULL.  q may remain NULL.
    */
   if ((dh->p == NULL && p == NULL)
       || (dh->g == NULL && g == NULL))
       return 0;

   if (p != NULL) {
       BN_free(dh->p);
       dh->p = p;
   }
   if (q != NULL) {
       BN_free(dh->q);
       dh->q = q;
   }
   if (g != NULL) {
       BN_free(dh->g);
       dh->g = g;
   }

   if (q != NULL) {
       dh->length = BN_num_bits(q);
   }

   return 1;
}
#endif
#endif /* defined(NETSNMP_USE_OPENSSL) */

/** TLS/DTLS certificatte support */
#if defined(NETSNMP_USE_OPENSSL) && defined(HAVE_LIBSSL) && !defined(NETSNMP_FEATURE_REMOVE_CERT_UTIL)
netsnmp_feature_require(container_free_all);

netsnmp_feature_child_of(openssl_cert_get_subjectAltNames, netsnmp_unused);
netsnmp_feature_child_of(openssl_ht2nid, netsnmp_unused);
netsnmp_feature_child_of(openssl_err_log, netsnmp_unused);
netsnmp_feature_child_of(cert_dump_names, netsnmp_unused);

#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/objects.h>

#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/library/cert_util.h>
#include <net-snmp/library/snmp_openssl.h>

static u_char have_started_already = 0;

/*
 * This code merely does openssl initialization so that multilpe
 * modules are safe to call netsnmp_init_openssl() for bootstrapping
 * without worrying about other callers that may have already done so.
 */
void netsnmp_init_openssl(void) {

    /* avoid duplicate calls */
    if (have_started_already)
        return;
    have_started_already = 1;

    DEBUGMSGTL(("snmp_openssl", "initializing\n"));

    /* Initializing OpenSSL */
#ifdef HAVE_SSL_LIBRARY_INIT
    SSL_library_init();
#endif
#ifdef HAVE_SSL_LOAD_ERROR_STRINGS
    SSL_load_error_strings();
#endif
#ifdef HAVE_ERR_LOAD_BIO_STRINGS
    ERR_load_BIO_strings();
#endif
#ifdef HAVE_OPENSSL_ADD_ALL_ALGORITHMS
    OpenSSL_add_all_algorithms();
#endif
}

/** netsnmp_openssl_cert_get_name: get subject name field from cert
 * @internal
 */
/** instead of exposing this function, make helper functions for each
 * field, like netsnmp_openssl_cert_get_commonName, below */
static char *
_cert_get_name(X509 *ocert, int which, char **buf, int *len, int flags)
{
    X509_NAME       *osubj_name;
    int              space;
    char            *buf_ptr;

    if ((NULL == ocert) || ((buf && !len) || (len && !buf)))
        return NULL;

    osubj_name = X509_get_subject_name(ocert);
    if (NULL == osubj_name) {
        DEBUGMSGT(("openssl:cert:name", "no subject name!\n"));
        return NULL;
    }

    /** see if buf is big enough, or allocate buf if none specified */
    space = X509_NAME_get_text_by_NID(osubj_name, which, NULL, 0);
    if (-1 == space)
        return NULL;
    ++space; /* for NUL */
    if (buf && *buf) {
        if (*len < space)
            return NULL;
        buf_ptr = *buf;
    }
    else {
        buf_ptr = calloc(1,space);
        if (!buf_ptr)
            return NULL;
    }
    space = X509_NAME_get_text_by_NID(osubj_name, which, buf_ptr, space);
    if (len)
        *len = space;

    return buf_ptr;
}

/** netsnmp_openssl_cert_get_subjectName: get subject name field from cert
 */
char *
netsnmp_openssl_cert_get_subjectName(X509 *ocert, char **buf, int *len)
{
    X509_NAME       *osubj_name;
    int              space;
    char            *buf_ptr;

    if ((NULL == ocert) || ((buf && !len) || (len && !buf)))
        return NULL;

    osubj_name = X509_get_subject_name(ocert);
    if (NULL == osubj_name) {
        DEBUGMSGT(("openssl:cert:name", "no subject name!\n"));
        return NULL;
    }

    if (buf) {
        buf_ptr = *buf;
        space = *len;
    }
    else {
        buf_ptr = NULL;
        space = 0;
    }
    buf_ptr = X509_NAME_oneline(osubj_name, buf_ptr, space);
    if (len)
        *len = strlen(buf_ptr);

    return buf_ptr;
}

/** netsnmp_openssl_cert_get_commonName: get commonName for cert.
 * if a pointer to a buffer and its length are specified, they will be
 * used. otherwise, a new buffer will be allocated, which the caller will
 * be responsbile for releasing.
 */
char *
netsnmp_openssl_cert_get_commonName(X509 *ocert, char **buf, int *len)
{
    return _cert_get_name(ocert, NID_commonName, buf, len, 0);
}

#ifndef NETSNMP_FEATURE_REMOVE_CERT_DUMP_NAMES

/** netsnmp_openssl_cert_dump_name: dump subject names in cert
 */
void
netsnmp_openssl_cert_dump_names(X509 *ocert)
{
    int              i, onid;
    X509_NAME_ENTRY *oname_entry;
    ASN1_STRING     *oname_value;
    X509_NAME       *osubj_name;
    const char      *prefix_short, *prefix_long;

    if (NULL == ocert)
        return;

    osubj_name = X509_get_subject_name(ocert);
    if (NULL == osubj_name) {
        DEBUGMSGT(("9:cert:dump:names", "no subject name!\n"));
        return;
    }

    for (i = 0; i < X509_NAME_entry_count(osubj_name); i++) {
        oname_entry = X509_NAME_get_entry(osubj_name, i);
        netsnmp_assert(NULL != oname_entry);
        oname_value = X509_NAME_ENTRY_get_data(oname_entry);

        if (oname_value->type != V_ASN1_PRINTABLESTRING)
            continue;

        /** get NID */
        onid = OBJ_obj2nid(X509_NAME_ENTRY_get_object(oname_entry));
        if (onid == NID_undef) {
            prefix_long = prefix_short = "UNKNOWN";
        }
        else {
            prefix_long = OBJ_nid2ln(onid);
            prefix_short = OBJ_nid2sn(onid);
        }

        DEBUGMSGT(("9:cert:dump:names",
                   "[%02d] NID type %d, ASN type %d\n", i, onid,
                   oname_value->type));
        DEBUGMSGT(("9:cert:dump:names", "%s/%s: '%s'\n", prefix_long,
                   prefix_short, ASN1_STRING_get0_data(oname_value)));
    }
}
#endif /* NETSNMP_FEATURE_REMOVE_CERT_DUMP_NAMES */

static char *
_cert_get_extension(X509_EXTENSION  *oext, char **buf, int *len, int flags)
{
    int              space;
    char            *buf_ptr = NULL;
    u_char          *data;
    BIO             *bio;
    
    if ((NULL == oext) || ((buf && !len) || (len && !buf)))
        return NULL;

    bio = BIO_new(BIO_s_mem());
    if (NULL == bio) {
        snmp_log(LOG_ERR, "could not get bio for extension\n");
        return NULL;
    }
    if (X509V3_EXT_print(bio, oext, 0, 0) != 1) {
        snmp_log(LOG_ERR, "could not print extension!\n");
        goto out;
    }

    space = BIO_get_mem_data(bio, &data);
    if (buf && *buf) {
        if (*len < space + 1) {
            snmp_log(LOG_ERR, "not enough buffer space to print extension\n");
            goto out;
        }
        buf_ptr = *buf;
    } else {
        buf_ptr = calloc(1, space + 1);
    }
    
    if (!buf_ptr) {
        snmp_log(LOG_ERR, "error in allocation for extension\n");
        goto out;
    }
    memcpy(buf_ptr, data, space);
    buf_ptr[space] = 0;
    if (len)
        *len = space;

out:
    BIO_vfree(bio);

    return buf_ptr;
}

/** netsnmp_openssl_cert_get_extension: get extension field from cert
 * @internal
 */
/** instead of exposing this function, make helper functions for each
 * field, like netsnmp_openssl_cert_get_subjectAltName, below */
X509_EXTENSION  *
_cert_get_extension_at(X509 *ocert, int pos, char **buf, int *len, int flags)
{
    X509_EXTENSION  *oext;

    if ((NULL == ocert) || ((buf && !len) || (len && !buf)))
        return NULL;

    oext = X509_get_ext(ocert,pos);
    if (NULL == oext) {
        snmp_log(LOG_ERR, "extension number %d not found!\n", pos);
        netsnmp_openssl_cert_dump_extensions(ocert);
        return NULL;
    }

    return oext;
}

/** netsnmp_openssl_cert_get_extension: get extension field from cert
 * @internal
 */
/** instead of exposing this function, make helper functions for each
 * field, like netsnmp_openssl_cert_get_subjectAltName, below */
static char *
_cert_get_extension_str_at(X509 *ocert, int pos, char **buf, int *len,
                           int flags)
{
    X509_EXTENSION  *oext;

    if ((NULL == ocert) || ((buf && !len) || (len && !buf)))
        return NULL;

    oext = X509_get_ext(ocert,pos);
    if (NULL == oext) {
        snmp_log(LOG_ERR, "extension number %d not found!\n", pos);
        netsnmp_openssl_cert_dump_extensions(ocert);
        return NULL;
    }

    return _cert_get_extension(oext, buf, len, flags);
}

/** _cert_get_extension_id: get extension field from cert
 * @internal
 */
/** instead of exposing this function, make helper functions for each
 * field, like netsnmp_openssl_cert_get_subjectAltName, below */
X509_EXTENSION *
_cert_get_extension_id(X509 *ocert, int which, char **buf, int *len, int flags)
{
    int pos;

    if ((NULL == ocert) || ((buf && !len) || (len && !buf)))
        return NULL;

    pos = X509_get_ext_by_NID(ocert,which,-1);
    if (pos < 0) {
        DEBUGMSGT(("openssl:cert:name", "no extension %d\n", which));
        return NULL;
    }

    return _cert_get_extension_at(ocert, pos, buf, len, flags);
}

#ifndef NETSNMP_FEATURE_REMOVE_OPENSSL_CERT_GET_SUBJECTALTNAMES
/** _cert_get_extension_id_str: get extension field from cert
 * @internal
 */
/** instead of exposing this function, make helper functions for each
 * field, like netsnmp_openssl_cert_get_subjectAltName, below */
static char *
_cert_get_extension_id_str(X509 *ocert, int which, char **buf, int *len,
                           int flags)
{
    int pos;

    if ((NULL == ocert) || ((buf && !len) || (len && !buf)))
        return NULL;

    pos = X509_get_ext_by_NID(ocert,which,-1);
    if (pos < 0) {
        DEBUGMSGT(("openssl:cert:name", "no extension %d\n", which));
        return NULL;
    }

    return _cert_get_extension_str_at(ocert, pos, buf, len, flags);
}
#endif /* NETSNMP_FEATURE_REMOVE_OPENSSL_CERT_GET_SUBJECTALTNAMES */

static char *
_extract_oname(const GENERAL_NAME *oname)
{
    char  ipbuf[60], *buf = NULL, *rtn = NULL;

    if (NULL == oname)
        return NULL;

    switch ( oname->type ) {
        case GEN_EMAIL:
        case GEN_DNS:
            /*case GEN_URI:*/
            ASN1_STRING_to_UTF8((unsigned char**)&buf, oname->d.ia5);
            if (buf)
                rtn = strdup(buf);
            break;

        case GEN_IPADD:
            if (oname->d.iPAddress->length == 4) {
                sprintf(ipbuf, "%d.%d.%d.%d", oname->d.iPAddress->data[0],
                        oname->d.iPAddress->data[1],
                        oname->d.iPAddress->data[2],
                        oname->d.iPAddress->data[3]);
                rtn = strdup(ipbuf);
            }
            else if ((oname->d.iPAddress->length == 16) ||
                     (oname->d.iPAddress->length == 20)) {
                char *pos = ipbuf;
                int   j;
                for(j = 0; j < oname->d.iPAddress->length; ++j) {
                    *pos++ = VAL2HEX(oname->d.iPAddress->data[j]);
                    *pos++ = ':';
                }
                *pos = '\0';
                rtn = strdup(ipbuf);
            }
            else
                NETSNMP_LOGONCE((LOG_WARNING, "unexpected ip addr length %d\n",
                       oname->d.iPAddress->length));

            break;
        default:
            DEBUGMSGT(("openssl:cert:san", "unknown/unsupported type %d\n",
                       oname->type));
            break;
    }
    DEBUGMSGT(("9:openssl:cert:san", "san=%s\n", buf));
    if (buf)
        OPENSSL_free(buf);

    return rtn;
}

#ifndef NETSNMP_FEATURE_REMOVE_OPENSSL_CERT_GET_SUBJECTALTNAMES
/** netsnmp_openssl_cert_get_subjectAltName: get subjectAltName for cert.
 * if a pointer to a buffer and its length are specified, they will be
 * used. otherwise, a new buffer will be allocated, which the caller will
 * be responsbile for releasing.
 */
char *
netsnmp_openssl_cert_get_subjectAltNames(X509 *ocert, char **buf, int *len)
{
    return _cert_get_extension_id_str(ocert, NID_subject_alt_name, buf, len, 0);
}
#endif /* NETSNMP_FEATURE_REMOVE_OPENSSL_CERT_GET_SUBJECTALTNAMES */

void
netsnmp_openssl_cert_dump_extensions(X509 *ocert)
{
    X509_EXTENSION  *extension;
    const char      *extension_name;
    char             buf[SNMP_MAXBUF], *buf_ptr = buf, *str, *lf;
    int              i, num_extensions, buf_len, nid;

    if (NULL == ocert)
        return;

    DEBUGIF("9:cert:dump") 
        ;
    else
        return; /* bail if debug not enabled */

    num_extensions = X509_get_ext_count(ocert);
    if (0 == num_extensions)
        DEBUGMSGT(("9:cert:dump", "    0 extensions\n"));
    for(i = 0; i < num_extensions; i++) {
        extension = X509_get_ext(ocert, i);
        nid = OBJ_obj2nid(X509_EXTENSION_get_object(extension));
        extension_name = OBJ_nid2sn(nid);
        buf_len = sizeof(buf);
        str = _cert_get_extension_str_at(ocert, i, &buf_ptr, &buf_len, 0);
        if (!str) {
            DEBUGMSGT(("9:cert:dump", "    %2d: %s\n", i,
                        extension_name));
            continue;
        }
        lf = strchr(str, '\n'); /* look for multiline strings */
        if (NULL != lf)
            *lf = '\0'; /* only log first line of multiline here */
        DEBUGMSGT(("9:cert:dump", "    %2d: %s = %s\n", i,
                   extension_name, str));
        while(lf) { /* log remaining parts of multiline string */
            str = ++lf;
            if (*str == '\0')
               break;
            lf = strchr(str, '\n');
            if (NULL == lf) 
                break;
            *lf = '\0';
            DEBUGMSGT(("9:cert:dump", "        %s\n", str));
        }
    }
}

static const struct {
    uint16_t nid;
    uint16_t ht;
} _htmap[] = {
    { 0, NS_HASH_NONE },
#ifdef NID_md5WithRSAEncryption
    { NID_md5WithRSAEncryption, NS_HASH_MD5 },
#endif
#ifdef NID_sha1WithRSAEncryption
    { NID_sha1WithRSAEncryption, NS_HASH_SHA1 },
#endif
#ifdef NID_ecdsa_with_SHA1
    { NID_ecdsa_with_SHA1, NS_HASH_SHA1 },
#endif
#ifdef NID_sha224WithRSAEncryption
    { NID_sha224WithRSAEncryption, NS_HASH_SHA224 },
#endif
#ifdef NID_ecdsa_with_SHA224
    { NID_ecdsa_with_SHA224, NS_HASH_SHA224 },
#endif
#ifdef NID_sha256WithRSAEncryption
    { NID_sha256WithRSAEncryption, NS_HASH_SHA256 },
#endif
#ifdef NID_ecdsa_with_SHA256
    { NID_ecdsa_with_SHA256, NS_HASH_SHA256 },
#endif
#ifdef NID_sha384WithRSAEncryption
    { NID_sha384WithRSAEncryption, NS_HASH_SHA384 },
#endif
#ifdef NID_ecdsa_with_SHA384
    { NID_ecdsa_with_SHA384, NS_HASH_SHA384 },
#endif
#ifdef NID_sha512WithRSAEncryption
    { NID_sha512WithRSAEncryption, NS_HASH_SHA512 },
#endif
#ifdef NID_ecdsa_with_SHA512
    { NID_ecdsa_with_SHA512, NS_HASH_SHA512 },
#endif
};

int
_nid2ht(int nid)
{
    int i;

    for (i = 0; i < sizeof(_htmap) / sizeof(_htmap[0]); i++) {
        if (_htmap[i].nid == nid)
            return _htmap[i].ht;
    }
    return 0;
}

#ifndef NETSNMP_FEATURE_REMOVE_OPENSSL_HT2NID
int
_ht2nid(int ht)
{
    int i;

    for (i = 0; i < sizeof(_htmap) / sizeof(_htmap[0]); i++) {
        if (_htmap[i].ht == ht)
            return _htmap[i].nid;
    }
    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_OPENSSL_HT2NID */

/**
 * returns allocated pointer caller must free.
 */
int
netsnmp_openssl_cert_get_hash_type(X509 *ocert)
{
    if (NULL == ocert)
        return 0;

    return _nid2ht(X509_get_signature_nid(ocert));
}

/**
 * returns allocated pointer caller must free.
 */
char *
netsnmp_openssl_cert_get_fingerprint(X509 *ocert, int alg)
{
    u_char           fingerprint[EVP_MAX_MD_SIZE];
    u_int            fingerprint_len, nid;
    const EVP_MD    *digest;
    char            *result = NULL;

    if (NULL == ocert)
        return NULL;

    nid = X509_get_signature_nid(ocert);
    DEBUGMSGT(("9:openssl:fingerprint", "alg %d, cert nid %d (%d)\n", alg, nid,
               _nid2ht(nid)));
        
    if ((-1 == alg) && nid)
        alg = _nid2ht(nid);

    switch (alg) {
        case NS_HASH_MD5:
            snmp_log(LOG_ERR, "hash type md5 not yet supported\n");
            return NULL;
            break;
        
        case NS_HASH_NONE:
            snmp_log(LOG_ERR, "hash type none not supported. using SHA1\n");
            /* FALLTHROUGH */

        case NS_HASH_SHA1:
            digest = EVP_sha1();
            break;

#ifdef HAVE_EVP_SHA224
        case NS_HASH_SHA224:
            digest = EVP_sha224();
            break;

        case NS_HASH_SHA256:
            digest = EVP_sha256();
            break;

#endif
#ifdef HAVE_EVP_SHA384
        case NS_HASH_SHA384:
            digest = EVP_sha384();
            break;

        case NS_HASH_SHA512:
            digest = EVP_sha512();
            break;
#endif

        default:
            snmp_log(LOG_ERR, "unknown hash algorithm %d\n", alg);
            return NULL;
    }

    if (_nid2ht(nid) != alg) {
        DEBUGMSGT(("openssl:fingerprint",
                   "WARNING: alg %d does not match cert alg %d\n",
                   alg, _nid2ht(nid)));
    }
    if (X509_digest(ocert,digest,fingerprint,&fingerprint_len)) {
        binary_to_hex(fingerprint, fingerprint_len, &result);
        if (NULL == result)
            snmp_log(LOG_ERR, "failed to hexify fingerprint\n");
        else
            DEBUGMSGT(("9:openssl:fingerprint", "fingerprint %s\n", result));
    }
    else
        snmp_log(LOG_ERR,"failed to compute fingerprint\n");

    return result;
}

/**
 * get container of netsnmp_cert_map structures from an ssl connection
 * certificate chain.
 */
netsnmp_container *
netsnmp_openssl_get_cert_chain(SSL *ssl)
{
    X509                  *ocert, *ocert_tmp;
    STACK_OF(X509)        *ochain;
    char                  *fingerprint;
    netsnmp_container     *chain_map;
    netsnmp_cert_map      *cert_map;
    int                    i, sk_num_res;

    netsnmp_assert_or_return(ssl != NULL, NULL);

    ocert = SSL_get_peer_certificate(ssl);
    if (!ocert) {
        /** no peer cert */
        snmp_log(LOG_ERR, "SSL peer has no certificate\n");
        return NULL;
    }
    DEBUGIF("9:cert:dump") {
        netsnmp_openssl_cert_dump_extensions(ocert);
    }

    /*
     * get fingerprint and save it
     */
    fingerprint = netsnmp_openssl_cert_get_fingerprint(ocert, -1);
    if (NULL == fingerprint)
        return NULL;

    /*
     * allocate cert map. Don't pass in fingerprint, since it would strdup
     * it and we've already got a copy.
     */
    cert_map = netsnmp_cert_map_alloc(NULL, ocert);
    if (NULL == cert_map) {
        free(fingerprint);
        return NULL;
    }
    cert_map->fingerprint = fingerprint;
    cert_map->hashType = netsnmp_openssl_cert_get_hash_type(ocert);

    chain_map = netsnmp_cert_map_container_create(0); /* no fp subcontainer */
    if (NULL == chain_map) {
        netsnmp_cert_map_free(cert_map);
        return NULL;
    }
    
    CONTAINER_INSERT(chain_map, cert_map);

    /** check for a chain to a CA */
    ochain = SSL_get_peer_cert_chain(ssl);
    sk_num_res = sk_num((const void *)ochain);
    if (!ochain || sk_num_res == 0) {
        DEBUGMSGT(("ssl:cert:chain", "peer has no cert chain\n"));
    }
    else {
        /*
         * loop over chain, adding fingerprint / cert for each
         */
        DEBUGMSGT(("ssl:cert:chain", "examining cert chain\n"));
        sk_num_res = sk_num((const void *)ochain);
        for(i = 0; i < sk_num_res; ++i) {
            ocert_tmp = (X509*)sk_value((const void *)ochain,i);
            fingerprint = netsnmp_openssl_cert_get_fingerprint(ocert_tmp, -1);
            if (NULL == fingerprint)
                break;
            cert_map = netsnmp_cert_map_alloc(NULL, ocert);
            if (NULL == cert_map) {
                free(fingerprint);
                break;
            }
            cert_map->fingerprint = fingerprint;
            cert_map->hashType = netsnmp_openssl_cert_get_hash_type(ocert_tmp);

            CONTAINER_INSERT(chain_map, cert_map);
        } /* chain loop */
        /*
         * if we broke out of loop before finishing, clean up
         */
        if (i < sk_num_res)
            CONTAINER_FREE_ALL(chain_map, NULL);
    } /* got peer chain */

    DEBUGMSGT(("ssl:cert:chain", "found %" NETSNMP_PRIz "u certs in chain\n",
               CONTAINER_SIZE(chain_map)));
    if (CONTAINER_SIZE(chain_map) == 0) {
        CONTAINER_FREE(chain_map);
        chain_map = NULL;
    }

    return chain_map;
}

/*
tlstmCertSANRFC822Name "Maps a subjectAltName's rfc822Name to a
                  tmSecurityName.  The local part of the rfc822Name is
                  passed unaltered but the host-part of the name must
                  be passed in lower case.
                  Example rfc822Name Field:  FooBar@Example.COM
                  is mapped to tmSecurityName: FooBar@example.com"

tlstmCertSANDNSName "Maps a subjectAltName's dNSName to a
                  tmSecurityName after first converting it to all
                  lower case."

tlstmCertSANIpAddress "Maps a subjectAltName's iPAddress to a
                  tmSecurityName by transforming the binary encoded
                  address as follows:
                  1) for IPv4 the value is converted into a decimal
                     dotted quad address (e.g. '192.0.2.1')
                  2) for IPv6 addresses the value is converted into a
                     32-character all lowercase hexadecimal string
                     without any colon separators.

                     Note that the resulting length is the maximum
                     length supported by the View-Based Access Control
                     Model (VACM).  Note that using both the Transport
                     Security Model's support for transport prefixes
                     (see the SNMP-TSM-MIB's
                     snmpTsmConfigurationUsePrefix object for details)
                     will result in securityName lengths that exceed
                     what VACM can handle."

tlstmCertSANAny "Maps any of the following fields using the
                  corresponding mapping algorithms:
                  | rfc822Name | tlstmCertSANRFC822Name |
                  | dNSName    | tlstmCertSANDNSName    |
                  | iPAddress  | tlstmCertSANIpAddress  |
                  The first matching subjectAltName value found in the
                  certificate of the above types MUST be used when
                  deriving the tmSecurityName."
*/
char *
_cert_get_san_type(X509 *ocert, int mapType)
{
    GENERAL_NAMES      *onames;
    const GENERAL_NAME *oname = NULL;
    char               *buf = NULL, *lower = NULL;
    int                 count, i;
 
    onames = (GENERAL_NAMES *)X509_get_ext_d2i(ocert, NID_subject_alt_name,
                                               NULL, NULL );
    if (NULL == onames)
        return NULL;

    count = sk_GENERAL_NAME_num(onames);

    for (i=0 ; i <count; ++i)  {
        oname = sk_GENERAL_NAME_value(onames, i);

        if (GEN_DNS == oname->type) {
            if ((TSNM_tlstmCertSANDNSName == mapType) ||
                (TSNM_tlstmCertSANAny == mapType)) {
                lower = buf = _extract_oname( oname );
                break;
            }
        }
        else if (GEN_IPADD == oname->type) {
            if ((TSNM_tlstmCertSANIpAddress == mapType) ||
                (TSNM_tlstmCertSANAny == mapType)) {
                buf = _extract_oname(oname);
                break;
            }
        }
        else if (GEN_EMAIL == oname->type) {
            if ((TSNM_tlstmCertSANRFC822Name == mapType) ||
                (TSNM_tlstmCertSANAny == mapType)) {
                buf = _extract_oname(oname);
                lower = strchr(buf, '@');
                if (NULL == lower) {
                    DEBUGMSGT(("openssl:secname:extract",
                               "email %s has no '@'!\n", buf));
                }
                else {
                    ++lower;
                    break;
                }
            }
            
        }
    } /* for loop */

    if (lower)
        for ( ; *lower; ++lower )
            *lower = tolower(0xFF & *lower);
    DEBUGMSGT(("openssl:cert:extension:san", "#%d type %d: %s\n", i,
               oname ? oname->type : -1, buf ? buf : "NULL"));

    return buf;
}

char *
netsnmp_openssl_extract_secname(netsnmp_cert_map *cert_map,
                                netsnmp_cert_map *peer_cert)
{
    char       *rtn = NULL;

    if (NULL == cert_map)
        return NULL;

    DEBUGMSGT(("openssl:secname:extract",
               "checking priority %d, san of type %d for %s\n",
               cert_map->priority, cert_map->mapType, peer_cert->fingerprint));

    switch(cert_map->mapType) {
        case TSNM_tlstmCertSpecified:
            rtn = strdup(cert_map->data);
            break;

        case TSNM_tlstmCertSANRFC822Name:
        case TSNM_tlstmCertSANDNSName:
        case TSNM_tlstmCertSANIpAddress:
        case TSNM_tlstmCertSANAny:
            if (NULL == peer_cert) {
                DEBUGMSGT(("openssl:secname:extract", "no peer cert for %s\n",
                           cert_map->fingerprint));
                break;
            }
            rtn = _cert_get_san_type(peer_cert->ocert, cert_map->mapType);
            if (NULL == rtn) {
                DEBUGMSGT(("openssl:secname:extract", "no san for %s\n",
                           peer_cert->fingerprint));
            }
            break;

        case TSNM_tlstmCertCommonName:
            rtn = netsnmp_openssl_cert_get_commonName(cert_map->ocert, NULL,
                                                       NULL);
            break;
        default:
            snmp_log(LOG_ERR, "cant extract secname for unknown map type %d\n",
                     cert_map->mapType);
            break;
    } /* switch mapType */

    if (rtn) {
        DEBUGMSGT(("openssl:secname:extract",
                   "found map %d, type %d for %s: %s\n", cert_map->priority,
                   cert_map->mapType, peer_cert->fingerprint, rtn));
        if (strlen(rtn) >32) {
            DEBUGMSGT(("openssl:secname:extract",
                       "secName longer than 32 chars! dropping...\n"));
            SNMP_FREE(rtn);
        }
    }
    else
        DEBUGMSGT(("openssl:secname:extract",
                   "no map of type %d for %s\n",
                   cert_map->mapType, peer_cert->fingerprint));
    return rtn;
}

int
netsnmp_openssl_cert_issued_by(X509 *issuer, X509 *cert)
{
    return (X509_check_issued(issuer, cert) == X509_V_OK);
}


#ifndef NETSNMP_FEATURE_REMOVE_OPENSSL_ERR_LOG
#ifndef ERR_GET_FUNC
/* removed in OpenSSL 3.0 */
#define ERR_GET_FUNC(e) -1
#endif

void
netsnmp_openssl_err_log(const char *prefix)
{
    unsigned long err;
    for (err = ERR_get_error(); err; err = ERR_get_error()) {
        snmp_log(LOG_ERR,"%s: %ld\n", prefix ? prefix: "openssl error", err);
        snmp_log(LOG_ERR, "library=%d, function=%d, reason=%d\n",
                 ERR_GET_LIB(err), ERR_GET_FUNC(err), ERR_GET_REASON(err));
    }
}
#endif /* NETSNMP_FEATURE_REMOVE_OPENSSL_ERR_LOG */

void
netsnmp_openssl_null_checks(SSL *ssl, int *null_auth, int *null_cipher)
{
    const SSL_CIPHER *cipher;
    char           tmp_buf[128], *cipher_alg, *auth_alg;

    if (null_auth)
        *null_auth = -1; /* unknown */
    if (null_cipher)
        *null_cipher = -1; /* unknown */
    if (NULL == ssl)
        return;

    cipher = SSL_get_current_cipher(ssl);
    if (NULL == cipher) {
        DEBUGMSGTL(("ssl:cipher", "no cipher yet\n"));
        return;
    }
    SSL_CIPHER_description(NETSNMP_REMOVE_CONST(SSL_CIPHER *, cipher), tmp_buf, sizeof(tmp_buf));
    /** no \n since tmp_buf already has one */
    DEBUGMSGTL(("ssl:cipher", "current cipher: %s", tmp_buf));

    /*
     * run "openssl ciphers -v eNULL" and "openssl ciphers -v aNULL"
     * to see NULL encryption/authentication algorithms. e.g.
     *
     * EXP-ADH-RC4-MD5 SSLv3 Kx=DH(512) Au=None Enc=RC4(40) Mac=MD5  export
     * NULL-SHA        SSLv3 Kx=RSA     Au=RSA  Enc=None    Mac=SHA1
     */
    if (null_cipher) {
        cipher_alg = strstr(tmp_buf, "Enc=");
        if (cipher_alg) {
            cipher_alg += 4;
            if (strncmp(cipher_alg,"None", 4) == 0)
                *null_cipher = 1;
            else
                *null_cipher = 0;
        }
    }
    if (null_auth) {
        auth_alg = strstr(tmp_buf, "Au=");
        if (auth_alg) {
            auth_alg += 3;
            if (strncmp(auth_alg,"None", 4) == 0)
                *null_auth = 1;
            else
                *null_auth = 0;
        }
    }
}

#ifndef HAVE_X509_GET_SIGNATURE_NID
int X509_get_signature_nid(const X509 *x)
{
    return OBJ_obj2nid(x->sig_alg->algorithm);
}
#endif

#ifndef HAVE_ASN1_STRING_GET0_DATA
const unsigned char *ASN1_STRING_get0_data(const ASN1_STRING *x)
{
    return x->data;
}
#endif

#ifndef HAVE_X509_NAME_ENTRY_GET_OBJECT
ASN1_OBJECT *X509_NAME_ENTRY_get_object(const X509_NAME_ENTRY *ne)
{
    if (ne == NULL)
        return NULL;
    return ne->object;
}
#endif

#ifndef HAVE_X509_NAME_ENTRY_GET_DATA
ASN1_STRING *X509_NAME_ENTRY_get_data(const X509_NAME_ENTRY *ne)
{
    if (ne == NULL)
        return NULL;
    return ne->value;
}
#endif

#ifndef HAVE_TLS_METHOD
const SSL_METHOD *TLS_method(void)
{
    return TLSv1_method();
}
#endif

#ifndef HAVE_DTLS_METHOD
const SSL_METHOD *DTLS_method(void)
{
    return DTLSv1_method();
}
#endif

#endif /* NETSNMP_USE_OPENSSL && HAVE_LIBSSL && !defined(NETSNMP_FEATURE_REMOVE_CERT_UTIL) */
