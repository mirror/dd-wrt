// SPDX-License-Identifier: Apache-2.0 AND MIT

/*
 * OQS OpenSSL 3 provider
 *
 * Code strongly inspired by OpenSSL DSA signature provider.
 *
 */

#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/rsa.h>
#include <openssl/types.h>
#include <openssl/x509.h>
#include <string.h>

#include "oqs/sig.h"
#include "oqs_prov.h"

// TBD: Review what we really need/want: For now go with OSSL settings:
#define OSSL_MAX_NAME_SIZE 50
#define OSSL_MAX_PROPQUERY_SIZE 256 /* Property query strings */
#define COMPOSITE_OID_PREFIX_LEN 26

#ifdef NDEBUG
#define OQS_SIG_PRINTF(a)
#define OQS_SIG_PRINTF2(a, b)
#define OQS_SIG_PRINTF3(a, b, c)
#else
#define OQS_SIG_PRINTF(a)                                                      \
    if (getenv("OQSSIG"))                                                      \
    printf(a)
#define OQS_SIG_PRINTF2(a, b)                                                  \
    if (getenv("OQSSIG"))                                                      \
    printf(a, b)
#define OQS_SIG_PRINTF3(a, b, c)                                               \
    if (getenv("OQSSIG"))                                                      \
    printf(a, b, c)
#endif // NDEBUG

static OSSL_FUNC_signature_newctx_fn oqs_sig_newctx;
static OSSL_FUNC_signature_sign_init_fn oqs_sig_sign_init;
static OSSL_FUNC_signature_verify_init_fn oqs_sig_verify_init;
static OSSL_FUNC_signature_sign_fn oqs_sig_sign;
static OSSL_FUNC_signature_verify_fn oqs_sig_verify;
static OSSL_FUNC_signature_digest_sign_init_fn oqs_sig_digest_sign_init;
static OSSL_FUNC_signature_digest_sign_update_fn
    oqs_sig_digest_signverify_update;
static OSSL_FUNC_signature_digest_sign_final_fn oqs_sig_digest_sign_final;
static OSSL_FUNC_signature_digest_verify_init_fn oqs_sig_digest_verify_init;
static OSSL_FUNC_signature_digest_verify_update_fn
    oqs_sig_digest_signverify_update;
static OSSL_FUNC_signature_digest_verify_final_fn oqs_sig_digest_verify_final;
static OSSL_FUNC_signature_freectx_fn oqs_sig_freectx;
static OSSL_FUNC_signature_dupctx_fn oqs_sig_dupctx;
static OSSL_FUNC_signature_get_ctx_params_fn oqs_sig_get_ctx_params;
static OSSL_FUNC_signature_gettable_ctx_params_fn oqs_sig_gettable_ctx_params;
static OSSL_FUNC_signature_set_ctx_params_fn oqs_sig_set_ctx_params;
static OSSL_FUNC_signature_settable_ctx_params_fn oqs_sig_settable_ctx_params;
static OSSL_FUNC_signature_get_ctx_md_params_fn oqs_sig_get_ctx_md_params;
static OSSL_FUNC_signature_gettable_ctx_md_params_fn
    oqs_sig_gettable_ctx_md_params;
static OSSL_FUNC_signature_set_ctx_md_params_fn oqs_sig_set_ctx_md_params;
static OSSL_FUNC_signature_settable_ctx_md_params_fn
    oqs_sig_settable_ctx_md_params;

// OIDS:
static int get_aid(unsigned char **oidbuf, const char *tls_name) {
    X509_ALGOR *algor = X509_ALGOR_new();
    int aidlen = 0;

    X509_ALGOR_set0(algor, OBJ_txt2obj(tls_name, 0), V_ASN1_UNDEF, NULL);

    aidlen = i2d_X509_ALGOR(algor, oidbuf);
    X509_ALGOR_free(algor);
    return (aidlen);
}

DECLARE_ASN1_FUNCTIONS(CompositeSignature)

ASN1_NDEF_SEQUENCE(CompositeSignature) =
    {
        ASN1_SIMPLE(CompositeSignature, sig1, ASN1_BIT_STRING),
        ASN1_SIMPLE(CompositeSignature, sig2, ASN1_BIT_STRING),
} ASN1_NDEF_SEQUENCE_END(CompositeSignature)

        IMPLEMENT_ASN1_FUNCTIONS(CompositeSignature)

    /*
     * What's passed as an actual key is defined by the KEYMGMT interface.
     */

    typedef struct {
    OSSL_LIB_CTX *libctx;
    char *propq;
    OQSX_KEY *sig;

    /*
     * Flag to determine if the hash function can be changed (1) or not (0)
     * Because it's dangerous to change during a DigestSign or DigestVerify
     * operation, this flag is cleared by their Init function, and set again
     * by their Final function.
     */
    unsigned int flag_allow_md : 1;

    char mdname[OSSL_MAX_NAME_SIZE];

    /* The Algorithm Identifier of the combined signature algorithm */
    unsigned char *aid;
    size_t aid_len;

    /* main digest */
    EVP_MD *md;
    EVP_MD_CTX *mdctx;
    size_t mdsize;
    // for collecting data if no MD is active:
    unsigned char *mddata;
    void *context_string;
    size_t context_string_length;
    int operation;
} PROV_OQSSIG_CTX;

static void *oqs_sig_newctx(void *provctx, const char *propq) {
    PROV_OQSSIG_CTX *poqs_sigctx;

    OQS_SIG_PRINTF2("OQS SIG provider: newctx called with propq %s\n", propq);

    poqs_sigctx = OPENSSL_zalloc(sizeof(PROV_OQSSIG_CTX));
    if (poqs_sigctx == NULL)
        return NULL;

    poqs_sigctx->libctx = ((PROV_OQS_CTX *)provctx)->libctx;
    if (propq != NULL && (poqs_sigctx->propq = OPENSSL_strdup(propq)) == NULL) {
        OPENSSL_free(poqs_sigctx);
        poqs_sigctx = NULL;
        ERR_raise(ERR_LIB_USER, ERR_R_MALLOC_FAILURE);
    }
    return poqs_sigctx;
}

static int oqs_sig_setup_md(PROV_OQSSIG_CTX *ctx, const char *mdname,
                            const char *mdprops) {
    OQS_SIG_PRINTF3("OQS SIG provider: setup_md called for MD %s (alg %s)\n",
                    mdname, ctx->sig->tls_name);
    if (mdprops == NULL)
        mdprops = ctx->propq;

    if (mdname != NULL) {
        EVP_MD *md = EVP_MD_fetch(ctx->libctx, mdname, mdprops);

        if ((md == NULL) || (EVP_MD_nid(md) == NID_undef)) {
            if (md == NULL)
                ERR_raise_data(ERR_LIB_USER, OQSPROV_R_INVALID_DIGEST,
                               "%s could not be fetched", mdname);
            EVP_MD_free(md);
            return 0;
        }

        EVP_MD_CTX_free(ctx->mdctx);
        ctx->mdctx = NULL;
        EVP_MD_free(ctx->md);
        ctx->md = NULL;

        if (ctx->aid)
            OPENSSL_free(ctx->aid);
        ctx->aid = NULL; // ensure next function allocates memory
        ctx->aid_len = get_aid(&(ctx->aid), ctx->sig->tls_name);

        ctx->md = md;
        OPENSSL_strlcpy(ctx->mdname, mdname, sizeof(ctx->mdname));
    }
    return 1;
}

static int oqs_sig_signverify_init(void *vpoqs_sigctx, void *voqssig,
                                   int operation) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    OQS_SIG_PRINTF("OQS SIG provider: signverify_init called\n");
    if (poqs_sigctx == NULL || voqssig == NULL || !oqsx_key_up_ref(voqssig))
        return 0;
    oqsx_key_free(poqs_sigctx->sig);
    poqs_sigctx->sig = voqssig;
    poqs_sigctx->operation = operation;
    poqs_sigctx->flag_allow_md = 1; /* change permitted until first use */
    if ((operation == EVP_PKEY_OP_SIGN && !poqs_sigctx->sig->privkey) ||
        (operation == EVP_PKEY_OP_VERIFY && !poqs_sigctx->sig->pubkey)) {
        ERR_raise(ERR_LIB_USER, OQSPROV_R_INVALID_KEY);
        return 0;
    }
    return 1;
}

static int oqs_sig_sign_init(void *vpoqs_sigctx, void *voqssig,
                             const OSSL_PARAM params[]) {
    OQS_SIG_PRINTF("OQS SIG provider: sign_init called\n");
    return oqs_sig_signverify_init(vpoqs_sigctx, voqssig, EVP_PKEY_OP_SIGN);
}

static int oqs_sig_verify_init(void *vpoqs_sigctx, void *voqssig,
                               const OSSL_PARAM params[]) {
    OQS_SIG_PRINTF("OQS SIG provider: verify_init called\n");
    return oqs_sig_signverify_init(vpoqs_sigctx, voqssig, EVP_PKEY_OP_VERIFY);
}

// this list need to be in order of the last number on the OID from the
// composite, the len of each value is COMPOSITE_OID_PREFIX_LEN
static const unsigned char *composite_OID_prefix[] = {
    /*
     * mldsa44_pss2048
     * id-MLDSA44-RSA2048-PSS-SHA256
     */
    (const unsigned char *)"060B6086480186FA6B50080101",

    /*
     * mldsa44_rsa2048
     * id-MLDSA44-RSA2048-PKCS15-SHA256
     */
    (const unsigned char *)"060B6086480186FA6B50080102",

    /*
     * mldsa44_ed25519
     * id-MLDSA44-Ed25519-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B50080103",

    /*
     * mldsa44_p256
     * id-MLDSA44-ECDSA-P256-SHA256
     */
    (const unsigned char *)"060B6086480186FA6B50080104",

    /*
     * mldsa44_bp256
     * id-MLDSA44-ECDSA-brainpoolP256r1-SHA256
     */
    (const unsigned char *)"060B6086480186FA6B50080105",

    /*
     * mldsa65_pss3072
     * id-MLDSA65-RSA3072-PSS-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B50080106",

    /*
     * mldsa65_rsa3072
     * id-MLDSA65-RSA3072-PKCS15-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B50080107",

    /*
     * mldsa65_p256
     * id-MLDSA65-ECDSA-P256-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B50080108",

    /*
     * mldsa65_bp256
     * id-MLDSA65-ECDSA-brainpoolP256r1-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B50080109",

    /*
     * mldsa65_ed25519
     * id-MLDSA65-Ed25519-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B5008010A",

    /*
     * mldsa87_p384
     * id-MLDSA87-ECDSA-P384-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B5008010B",

    /*
     * mldsa87_bp384
     * id-MLDSA87-ECDSA-brainpoolP384r1-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B5008010C",

    /*
     * mldsa87_ed448
     * id-MLDSA87-Ed448-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B5008010D",

    /*
     * falcon512_p256
     * id-Falon512-ECDSA-P256-SHA256
     */
    (const unsigned char *)"060B6086480186FA6B5008010E",

    /*
     * falcon512_p256
     * id-Falcon512-ECDSA-brainpoolP256r1-SHA256
     */
    (const unsigned char *)"060B6086480186FA6B5008010F",

    /*
     * falcon512_ed25519
     * id-Falcon512-Ed25519-SHA512
     */
    (const unsigned char *)"060B6086480186FA6B50080110",
};

/*put the chars on in into memory on out*/
void composite_prefix_conversion(char *out, const unsigned char *in) {
    int temp;
    for (int i = 0; i < COMPOSITE_OID_PREFIX_LEN / 2; i++) {
        temp = OPENSSL_hexchar2int(in[2 * i]);
        temp = temp * 16;
        temp += OPENSSL_hexchar2int(in[2 * i + 1]);
        out[i] = (unsigned char)temp;
    }
}

/* On entry to this function, data to be signed (tbs) might have been hashed
 * already: this would be the case if poqs_sigctx->mdctx != NULL; if that is
 * NULL, we have to hash in case of hybrid signatures
 */
static int oqs_sig_sign(void *vpoqs_sigctx, unsigned char *sig, size_t *siglen,
                        size_t sigsize, const unsigned char *tbs,
                        size_t tbslen) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;
    OQSX_KEY *oqsxkey = poqs_sigctx->sig;
    OQS_SIG *oqs_key = poqs_sigctx->sig->oqsx_provider_ctx.oqsx_qs_ctx.sig;
    EVP_PKEY *oqs_key_classic = NULL;
    EVP_PKEY *cmp_key_classic = NULL;
    EVP_PKEY *evpkey = oqsxkey->classical_pkey;
    EVP_PKEY_CTX *classical_ctx_sign = NULL;

    OQS_SIG_PRINTF2("OQS SIG provider: sign called for %ld bytes\n", tbslen);

    int is_hybrid = (oqsxkey->keytype == KEY_TYPE_HYB_SIG);
    int is_composite = (oqsxkey->keytype == KEY_TYPE_CMP_SIG);
    size_t max_sig_len = 0;
    size_t classical_sig_len = 0, oqs_sig_len = 0;
    size_t actual_classical_sig_len = 0;
    size_t index = 0;
    int rv = 0;

    if (!oqsxkey || !(oqs_key || oqs_key_classic) || !oqsxkey->privkey) {
        ERR_raise(ERR_LIB_USER, OQSPROV_R_NO_PRIVATE_KEY);
        return rv;
    }

    if (is_composite) {
        max_sig_len = oqsx_key_maxsize(oqsxkey);
    } else {
        max_sig_len += oqs_key->length_signature;
    }

    if (is_hybrid) {
        actual_classical_sig_len = oqsxkey->evp_info->length_signature;
        max_sig_len += (SIZE_OF_UINT32 + actual_classical_sig_len);
    }

    if (sig == NULL) {
        *siglen = max_sig_len;
        OQS_SIG_PRINTF2("OQS SIG provider: sign test returning size %ld\n",
                        *siglen);
        return 1;
    }
    if (*siglen < max_sig_len) {
        ERR_raise(ERR_LIB_USER, OQSPROV_R_BUFFER_LENGTH_WRONG);
        return rv;
    }

    if (is_hybrid) {
        if ((classical_ctx_sign = EVP_PKEY_CTX_new(evpkey, NULL)) == NULL ||
            EVP_PKEY_sign_init(classical_ctx_sign) <= 0) {
            ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
            goto endsign;
        }
        if (oqsxkey->evp_info->keytype == EVP_PKEY_RSA) {
            if (EVP_PKEY_CTX_set_rsa_padding(classical_ctx_sign,
                                             RSA_PKCS1_PADDING) <= 0) {
                ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                goto endsign;
            }
        }

        /* unconditionally hash to be in line with oqs-openssl111:
         * uncomment the following line if using pre-performed hash:
         * if (poqs_sigctx->mdctx == NULL) { // hashing not yet done
         */
        const EVP_MD *classical_md;
        int digest_len;
        unsigned char digest[SHA512_DIGEST_LENGTH]; /* init with max length */

        /* classical schemes can't sign arbitrarily large data; we hash it
         * first
         */
        switch (oqs_key->claimed_nist_level) {
        case 1:
            classical_md = EVP_sha256();
            digest_len = SHA256_DIGEST_LENGTH;
            SHA256(tbs, tbslen, (unsigned char *)&digest);
            break;
        case 2:
        case 3:
            classical_md = EVP_sha384();
            digest_len = SHA384_DIGEST_LENGTH;
            SHA384(tbs, tbslen, (unsigned char *)&digest);
            break;
        case 4:
        case 5:
        default:
            classical_md = EVP_sha512();
            digest_len = SHA512_DIGEST_LENGTH;
            SHA512(tbs, tbslen, (unsigned char *)&digest);
            break;
        }
        if ((EVP_PKEY_CTX_set_signature_md(classical_ctx_sign, classical_md) <=
             0) ||
            (EVP_PKEY_sign(classical_ctx_sign, sig + SIZE_OF_UINT32,
                           &actual_classical_sig_len, digest,
                           digest_len) <= 0)) {
            ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
            goto endsign;
        }
        /* activate in case we want to use pre-performed hashes:
         * }
         * else { // hashing done before; just sign:
         *     if (EVP_PKEY_sign(classical_ctx_sign, sig + SIZE_OF_UINT32,
         * &actual_classical_sig_len, tbs, tbslen) <= 0) {
         *       ERR_raise(ERR_LIB_USER, OQSPROV_R_SIGNING_FAILED);
         *       goto endsign;
         *     }
         *  }
         */
        if (actual_classical_sig_len > oqsxkey->evp_info->length_signature) {
            /* sig is bigger than expected */
            ERR_raise(ERR_LIB_USER, OQSPROV_R_BUFFER_LENGTH_WRONG);
            goto endsign;
        }
        ENCODE_UINT32(sig, actual_classical_sig_len);
        classical_sig_len = SIZE_OF_UINT32 + actual_classical_sig_len;
        index += classical_sig_len;
    }

    if (is_composite) {
        unsigned char *buf;
        int i;
        int nid = OBJ_sn2nid(oqsxkey->tls_name);
        int comp_idx = get_composite_idx(oqsxkey->tls_name);
        if (comp_idx == -1)
            goto endsign;
        const unsigned char *oid_prefix = composite_OID_prefix[comp_idx - 1];
        char *final_tbs;
        CompositeSignature *compsig = CompositeSignature_new();
        size_t final_tbslen = COMPOSITE_OID_PREFIX_LEN /
                              2; // COMPOSITE_OID_PREFIX_LEN stores the size of
                                 // the *char, but the prefix will be on memory,
                                 // so each 2 chars will translate into one byte
        int aux = 0;
        unsigned char *tbs_hash;

        // prepare the pre hash
        for (i = 0; i < oqsxkey->numkeys; i++) {
            char *name;
            char *upcase_name;
            if ((name = get_cmpname(nid, i)) == NULL) {
                ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                CompositeSignature_free(compsig);
                goto endsign;
            }
            upcase_name = get_oqsname_fromtls(name);

            if ((upcase_name != 0) &&
                    ((!strcmp(upcase_name, OQS_SIG_alg_ml_dsa_65)) ||
                     (!strcmp(upcase_name, OQS_SIG_alg_ml_dsa_87))) ||
                (name[0] == 'e')) {
                aux = 1;
                OPENSSL_free(name);
                break;
            }
            OPENSSL_free(name);
        }
        switch (aux) {
        case 0:
            tbs_hash = OPENSSL_malloc(SHA256_DIGEST_LENGTH);
            SHA256(tbs, tbslen, tbs_hash);
            final_tbslen += SHA256_DIGEST_LENGTH;
            break;
        case 1:
            tbs_hash = OPENSSL_malloc(SHA512_DIGEST_LENGTH);
            SHA512(tbs, tbslen, tbs_hash);
            final_tbslen += SHA512_DIGEST_LENGTH;
            break;
        default:
            ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
            CompositeSignature_free(compsig);
            goto endsign;
        }
        final_tbs = OPENSSL_malloc(final_tbslen);
        composite_prefix_conversion(final_tbs, oid_prefix);
        memcpy(final_tbs + COMPOSITE_OID_PREFIX_LEN / 2, tbs_hash,
               final_tbslen - COMPOSITE_OID_PREFIX_LEN / 2);
        OPENSSL_free(tbs_hash);

        // sign
        for (i = 0; i < oqsxkey->numkeys; i++) {
            char *name;
            if ((name = get_cmpname(nid, i)) == NULL) {
                ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                CompositeSignature_free(compsig);
                OPENSSL_free(final_tbs);
                goto endsign;
            }

            if (get_oqsname_fromtls(name)) { // PQC signing
                oqs_sig_len = oqsxkey->oqsx_provider_ctx.oqsx_qs_ctx.sig
                                  ->length_signature;
                buf = OPENSSL_malloc(oqs_sig_len);
#if !defined OQS_VERSION_MINOR ||                                              \
    (OQS_VERSION_MAJOR == 0 && OQS_VERSION_MINOR < 12)
                if (OQS_SIG_sign(oqs_key, buf, &oqs_sig_len,
                                 (const unsigned char *)final_tbs, final_tbslen,
                                 oqsxkey->comp_privkey[i]) != OQS_SUCCESS) {
#else
                if (OQS_SIG_sign_with_ctx_str(
                        oqs_key, buf, &oqs_sig_len,
                        (const unsigned char *)final_tbs, final_tbslen,
                        poqs_sigctx->context_string,
                        poqs_sigctx->context_string_length,
                        oqsxkey->comp_privkey[i]) != OQS_SUCCESS) {
#endif
                    ERR_raise(ERR_LIB_USER, OQSPROV_R_SIGNING_FAILED);
                    CompositeSignature_free(compsig);
                    OPENSSL_free(final_tbs);
                    OPENSSL_free(name);
                    OPENSSL_free(buf);
                    goto endsign;
                }
            } else { // sign non PQC key on oqs_key
                oqs_key_classic = oqsxkey->classical_pkey;
                oqs_sig_len = oqsxkey->oqsx_provider_ctx.oqsx_evp_ctx->evp_info
                                  ->length_signature;
                buf = OPENSSL_malloc(oqs_sig_len);
                const EVP_MD *classical_md;
                int digest_len;
                unsigned char digest[SHA512_DIGEST_LENGTH]; /* init with max
                                                               length */

                if (name[0] == 'e') { // ed25519 or ed448
                    EVP_MD_CTX *evp_ctx = EVP_MD_CTX_new();
                    if ((EVP_DigestSignInit(evp_ctx, NULL, NULL, NULL,
                                            oqs_key_classic) <= 0) ||
                        (EVP_DigestSign(evp_ctx, buf, &oqs_sig_len,
                                        (const unsigned char *)final_tbs,
                                        final_tbslen) <= 0)) {
                        ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                        CompositeSignature_free(compsig);
                        OPENSSL_free(final_tbs);
                        OPENSSL_free(name);
                        EVP_MD_CTX_free(evp_ctx);
                        OPENSSL_free(buf);
                        goto endsign;
                    }
                    EVP_MD_CTX_free(evp_ctx);
                } else {
                    if ((classical_ctx_sign =
                             EVP_PKEY_CTX_new(oqs_key_classic, NULL)) == NULL ||
                        (EVP_PKEY_sign_init(classical_ctx_sign) <= 0)) {
                        ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                        CompositeSignature_free(compsig);
                        OPENSSL_free(final_tbs);
                        OPENSSL_free(name);
                        OPENSSL_free(buf);
                        goto endsign;
                    }

                    if (!strncmp(name, "pss", 3)) {
                        int salt;
                        const EVP_MD *pss_mgf1;
                        if (!strncmp(name, "pss3072", 7)) {
                            salt = 64;
                            pss_mgf1 = EVP_sha512();
                        } else {
                            if (!strncmp(name, "pss2048", 7)) {
                                salt = 32;
                                pss_mgf1 = EVP_sha256();
                            } else {
                                ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                                CompositeSignature_free(compsig);
                                OPENSSL_free(final_tbs);
                                OPENSSL_free(name);
                                OPENSSL_free(buf);
                                goto endsign;
                            }
                        }
                        if ((EVP_PKEY_CTX_set_rsa_padding(
                                 classical_ctx_sign, RSA_PKCS1_PSS_PADDING) <=
                             0) ||
                            (EVP_PKEY_CTX_set_rsa_pss_saltlen(
                                 classical_ctx_sign, salt) <= 0) ||
                            (EVP_PKEY_CTX_set_rsa_mgf1_md(classical_ctx_sign,
                                                          pss_mgf1) <= 0)) {
                            ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                            CompositeSignature_free(compsig);
                            OPENSSL_free(final_tbs);
                            OPENSSL_free(name);
                            OPENSSL_free(buf);
                            goto endsign;
                        }
                    } else if (oqsxkey->oqsx_provider_ctx.oqsx_evp_ctx->evp_info
                                   ->keytype == EVP_PKEY_RSA) {
                        if (EVP_PKEY_CTX_set_rsa_padding(
                                classical_ctx_sign, RSA_PKCS1_PADDING) <= 0) {
                            ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                            CompositeSignature_free(compsig);
                            OPENSSL_free(final_tbs);
                            OPENSSL_free(name);
                            OPENSSL_free(buf);
                            goto endsign;
                        }
                    }
                    if (comp_idx < 6) {
                        classical_md = EVP_sha256();
                        digest_len = SHA256_DIGEST_LENGTH;
                        SHA256((const unsigned char *)final_tbs, final_tbslen,
                               (unsigned char *)&digest);
                    } else {
                        classical_md = EVP_sha512();
                        digest_len = SHA512_DIGEST_LENGTH;
                        SHA512((const unsigned char *)final_tbs, final_tbslen,
                               (unsigned char *)&digest);
                    }

                    if ((EVP_PKEY_CTX_set_signature_md(classical_ctx_sign,
                                                       classical_md) <= 0) ||
                        (EVP_PKEY_sign(classical_ctx_sign, buf, &oqs_sig_len,
                                       digest, digest_len) <= 0)) {
                        ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                        CompositeSignature_free(compsig);
                        OPENSSL_free(final_tbs);
                        OPENSSL_free(name);
                        OPENSSL_free(buf);
                        goto endsign;
                    }

                    if (oqs_sig_len > oqsxkey->oqsx_provider_ctx.oqsx_evp_ctx
                                          ->evp_info->length_signature) {
                        /* sig is bigger than expected */
                        ERR_raise(ERR_LIB_USER, OQSPROV_R_BUFFER_LENGTH_WRONG);
                        CompositeSignature_free(compsig);
                        OPENSSL_free(final_tbs);
                        OPENSSL_free(name);
                        OPENSSL_free(buf);
                        goto endsign;
                    }
                }
            }

            if (i == 0) {
                compsig->sig1->data = OPENSSL_memdup(buf, oqs_sig_len);
                compsig->sig1->length = oqs_sig_len;
                compsig->sig1->flags =
                    8; // set as 8 to not check for unused bits
            } else {
                compsig->sig2->data = OPENSSL_memdup(buf, oqs_sig_len);
                compsig->sig2->length = oqs_sig_len;
                compsig->sig2->flags =
                    8; // set as 8 to not check for unused bits
            }

            OPENSSL_free(buf);
            OPENSSL_free(name);
        }
        oqs_sig_len = i2d_CompositeSignature(compsig, &sig);

        CompositeSignature_free(compsig);
        OPENSSL_free(final_tbs);
#if !defined OQS_VERSION_MINOR ||                                              \
    (OQS_VERSION_MAJOR == 0 && OQS_VERSION_MINOR < 12)
    } else if (OQS_SIG_sign(oqs_key, sig + index, &oqs_sig_len, tbs, tbslen,
#else
    } else if (OQS_SIG_sign_with_ctx_str(
                   oqs_key, sig + index, &oqs_sig_len, tbs, tbslen,
                   poqs_sigctx->context_string,
                   poqs_sigctx->context_string_length,
#endif
                            oqsxkey->comp_privkey[oqsxkey->numkeys - 1]) !=
               OQS_SUCCESS) {
        ERR_raise(ERR_LIB_USER, OQSPROV_R_SIGNING_FAILED);
        goto endsign;
    }

    *siglen = classical_sig_len + oqs_sig_len;
    OQS_SIG_PRINTF2("OQS SIG provider: signing completes with size %ld\n",
                    *siglen);
    rv = 1; /* success */

endsign:
    if (classical_ctx_sign) {
        EVP_PKEY_CTX_free(classical_ctx_sign);
    }
    return rv;
}

static int oqs_sig_verify(void *vpoqs_sigctx, const unsigned char *sig,
                          size_t siglen, const unsigned char *tbs,
                          size_t tbslen) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;
    OQSX_KEY *oqsxkey = poqs_sigctx->sig;
    OQS_SIG *oqs_key = poqs_sigctx->sig->oqsx_provider_ctx.oqsx_qs_ctx.sig;
    EVP_PKEY *evpkey = oqsxkey->classical_pkey;
    EVP_PKEY_CTX *classical_ctx_sign = NULL;
    EVP_PKEY_CTX *ctx_verify = NULL;
    int is_hybrid = (oqsxkey->keytype == KEY_TYPE_HYB_SIG);
    int is_composite = (oqsxkey->keytype == KEY_TYPE_CMP_SIG);
    size_t classical_sig_len = 0, oqs_sig_len = 0;
    size_t index = 0;
    int rv = 0;
    ASN1_BIT_STRING *comp_sig;

    OQS_SIG_PRINTF3("OQS SIG provider: verify called with siglen %ld bytes and "
                    "tbslen %ld\n",
                    siglen, tbslen);

    if (!oqsxkey || !oqs_key || !oqsxkey->pubkey || sig == NULL ||
        (tbs == NULL && tbslen > 0)) {
        ERR_raise(ERR_LIB_USER, OQSPROV_R_WRONG_PARAMETERS);
        goto endverify;
    }

    if (is_hybrid) {
        const EVP_MD *classical_md;
        uint32_t actual_classical_sig_len = 0;
        int digest_len;
        unsigned char digest[SHA512_DIGEST_LENGTH]; /* init with max length */
        size_t max_pq_sig_len =
            oqsxkey->oqsx_provider_ctx.oqsx_qs_ctx.sig->length_signature;
        size_t max_classical_sig_len =
            oqsxkey->oqsx_provider_ctx.oqsx_evp_ctx->evp_info->length_signature;

        if ((ctx_verify = EVP_PKEY_CTX_new(oqsxkey->classical_pkey, NULL)) ==
                NULL ||
            EVP_PKEY_verify_init(ctx_verify) <= 0) {
            ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
            goto endverify;
        }
        if (oqsxkey->evp_info->keytype == EVP_PKEY_RSA) {
            if (EVP_PKEY_CTX_set_rsa_padding(ctx_verify, RSA_PKCS1_PADDING) <=
                0) {
                ERR_raise(ERR_LIB_USER, OQSPROV_R_WRONG_PARAMETERS);
                goto endverify;
            }
        }
        if (siglen > SIZE_OF_UINT32) {
            size_t actual_pq_sig_len = 0;
            DECODE_UINT32(actual_classical_sig_len, sig);
            actual_pq_sig_len =
                siglen - SIZE_OF_UINT32 - actual_classical_sig_len;
            if (siglen <= (SIZE_OF_UINT32 + actual_classical_sig_len) ||
                actual_classical_sig_len > max_classical_sig_len ||
                actual_pq_sig_len > max_pq_sig_len) {
                ERR_raise(ERR_LIB_USER, OQSPROV_R_INVALID_ENCODING);
                goto endverify;
            }
        } else {
            ERR_raise(ERR_LIB_USER, OQSPROV_R_INVALID_ENCODING);
            goto endverify;
        }

        /* same as with sign: activate if pre-existing hashing to be used:
         *  if (poqs_sigctx->mdctx == NULL) { // hashing not yet done
         */
        switch (oqs_key->claimed_nist_level) {
        case 1:
            classical_md = EVP_sha256();
            digest_len = SHA256_DIGEST_LENGTH;
            SHA256(tbs, tbslen, (unsigned char *)&digest);
            break;
        case 2:
        case 3:
            classical_md = EVP_sha384();
            digest_len = SHA384_DIGEST_LENGTH;
            SHA384(tbs, tbslen, (unsigned char *)&digest);
            break;
        case 4:
        case 5:
        default:
            classical_md = EVP_sha512();
            digest_len = SHA512_DIGEST_LENGTH;
            SHA512(tbs, tbslen, (unsigned char *)&digest);
            break;
        }
        if ((EVP_PKEY_CTX_set_signature_md(ctx_verify, classical_md) <= 0) ||
            (EVP_PKEY_verify(ctx_verify, sig + SIZE_OF_UINT32,
                             actual_classical_sig_len, digest,
                             digest_len) <= 0)) {
            ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
            goto endverify;
        } else {
            OQS_SIG_PRINTF("OQS SIG: classic verification OK\n");
        }
        /* activate for using pre-existing digest:
         * }
         *  else { // hashing already done:
         *     if (EVP_PKEY_verify(ctx_verify, sig + SIZE_OF_UINT32,
         * actual_classical_sig_len, tbs, tbslen) <= 0) {
         *       ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
         *       goto endverify;
         *     }
         *  }
         */
        classical_sig_len = SIZE_OF_UINT32 + actual_classical_sig_len;
        index += classical_sig_len;
    }
    if (is_composite) {
        CompositeSignature *compsig;
        int i;
        int nid = OBJ_sn2nid(oqsxkey->tls_name);
        int comp_idx = get_composite_idx(oqsxkey->tls_name);
        if (comp_idx == -1)
            goto endverify;
        unsigned char *buf;
        size_t buf_len;
        const unsigned char *oid_prefix = composite_OID_prefix[comp_idx - 1];
        char *final_tbs;
        size_t final_tbslen = COMPOSITE_OID_PREFIX_LEN / 2;
        int aux = 0;
        unsigned char *tbs_hash;

        if ((compsig = d2i_CompositeSignature(NULL, &sig, siglen)) == NULL) {
            ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
            CompositeSignature_free(compsig);
            goto endverify;
        }

        // prepare the pre hash
        for (i = 0; i < oqsxkey->numkeys; i++) {
            char *name;
            char *upcase_name;
            if ((name = get_cmpname(nid, i)) == NULL) {
                ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
                CompositeSignature_free(compsig);
                goto endverify;
            }
            upcase_name = get_oqsname_fromtls(name);

            if ((upcase_name != 0) &&
                    ((!strcmp(upcase_name, OQS_SIG_alg_ml_dsa_65)) ||
                     (!strcmp(upcase_name, OQS_SIG_alg_ml_dsa_87))) ||
                (name[0] == 'e')) {
                aux = 1;
                OPENSSL_free(name);
                break;
            }
            OPENSSL_free(name);
        }
        switch (aux) {
        case 0:
            tbs_hash = OPENSSL_malloc(SHA256_DIGEST_LENGTH);
            SHA256(tbs, tbslen, tbs_hash);
            final_tbslen += SHA256_DIGEST_LENGTH;
            break;
        case 1:
            tbs_hash = OPENSSL_malloc(SHA512_DIGEST_LENGTH);
            SHA512(tbs, tbslen, tbs_hash);
            final_tbslen += SHA512_DIGEST_LENGTH;
            break;
        default:
            ERR_raise(ERR_LIB_USER, ERR_R_FATAL);
            CompositeSignature_free(compsig);
            goto endverify;
        }
        final_tbs = OPENSSL_malloc(final_tbslen);
        composite_prefix_conversion(final_tbs, oid_prefix);
        memcpy(final_tbs + COMPOSITE_OID_PREFIX_LEN / 2, tbs_hash,
               final_tbslen - COMPOSITE_OID_PREFIX_LEN / 2);
        OPENSSL_free(tbs_hash);

        // verify
        for (i = 0; i < oqsxkey->numkeys; i++) {
            if (i == 0) {
                buf = compsig->sig1->data;
                buf_len = compsig->sig1->length;
            } else {
                buf = compsig->sig2->data;
                buf_len = compsig->sig2->length;
            }

            char *name;
            if ((name = get_cmpname(nid, i)) == NULL) {
                ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
                CompositeSignature_free(compsig);
                OPENSSL_free(final_tbs);
                goto endverify;
            }

            if (get_oqsname_fromtls(name)) {
#if !defined OQS_VERSION_MINOR ||                                              \
    (OQS_VERSION_MAJOR == 0 && OQS_VERSION_MINOR < 12)
                if (OQS_SIG_verify(oqs_key, (const unsigned char *)final_tbs,
                                   final_tbslen, buf, buf_len,
                                   oqsxkey->comp_pubkey[i]) != OQS_SUCCESS) {
#else
                if (OQS_SIG_verify_with_ctx_str(
                        oqs_key, (const unsigned char *)final_tbs, final_tbslen,
                        buf, buf_len, poqs_sigctx->context_string,
                        poqs_sigctx->context_string_length,
                        oqsxkey->comp_pubkey[i]) != OQS_SUCCESS) {
#endif
                    ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
                    OPENSSL_free(name);
                    CompositeSignature_free(compsig);
                    OPENSSL_free(final_tbs);
                    goto endverify;
                }
            } else {
                const EVP_MD *classical_md;
                int digest_len;
                int aux;
                unsigned char digest[SHA512_DIGEST_LENGTH]; /* init with max
                                                               length */

                if (name[0] == 'e') { // ed25519 or ed448
                    EVP_MD_CTX *evp_ctx = EVP_MD_CTX_new();
                    if ((EVP_DigestVerifyInit(evp_ctx, NULL, NULL, NULL,
                                              oqsxkey->classical_pkey) <= 0) ||
                        (EVP_DigestVerify(evp_ctx, buf, buf_len,
                                          (const unsigned char *)final_tbs,
                                          final_tbslen) <= 0)) {
                        ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
                        OPENSSL_free(name);
                        EVP_MD_CTX_free(evp_ctx);
                        CompositeSignature_free(compsig);
                        OPENSSL_free(final_tbs);
                        goto endverify;
                    }
                    EVP_MD_CTX_free(evp_ctx);
                } else {
                    if (((ctx_verify = EVP_PKEY_CTX_new(oqsxkey->classical_pkey,
                                                        NULL)) == NULL) ||
                        (EVP_PKEY_verify_init(ctx_verify) <= 0)) {
                        ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
                        OPENSSL_free(name);
                        CompositeSignature_free(compsig);
                        OPENSSL_free(final_tbs);
                        goto endverify;
                    }
                    if (!strncmp(name, "pss", 3)) {
                        int salt;
                        const EVP_MD *pss_mgf1;
                        if (!strncmp(name, "pss3072", 7)) {
                            salt = 64;
                            pss_mgf1 = EVP_sha512();
                        } else {
                            if (!strncmp(name, "pss2048", 7)) {
                                salt = 32;
                                pss_mgf1 = EVP_sha256();
                            } else {
                                ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
                                OPENSSL_free(name);
                                CompositeSignature_free(compsig);
                                OPENSSL_free(final_tbs);
                                goto endverify;
                            }
                        }
                        if ((EVP_PKEY_CTX_set_rsa_padding(
                                 ctx_verify, RSA_PKCS1_PSS_PADDING) <= 0) ||
                            (EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx_verify,
                                                              salt) <= 0) ||
                            (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx_verify,
                                                          pss_mgf1) <= 0)) {
                            ERR_raise(ERR_LIB_USER, OQSPROV_R_WRONG_PARAMETERS);
                            OPENSSL_free(name);
                            CompositeSignature_free(compsig);
                            OPENSSL_free(final_tbs);
                            goto endverify;
                        }
                    } else if (oqsxkey->oqsx_provider_ctx.oqsx_evp_ctx->evp_info
                                   ->keytype == EVP_PKEY_RSA) {
                        if (EVP_PKEY_CTX_set_rsa_padding(
                                ctx_verify, RSA_PKCS1_PADDING) <= 0) {
                            ERR_raise(ERR_LIB_USER, OQSPROV_R_WRONG_PARAMETERS);
                            OPENSSL_free(name);
                            CompositeSignature_free(compsig);
                            OPENSSL_free(final_tbs);
                            goto endverify;
                        }
                    }
                    if (comp_idx < 6) {
                        classical_md = EVP_sha256();
                        digest_len = SHA256_DIGEST_LENGTH;
                        SHA256((const unsigned char *)final_tbs, final_tbslen,
                               (unsigned char *)&digest);
                    } else {
                        classical_md = EVP_sha512();
                        digest_len = SHA512_DIGEST_LENGTH;
                        SHA512((const unsigned char *)final_tbs, final_tbslen,
                               (unsigned char *)&digest);
                    }

                    if ((EVP_PKEY_CTX_set_signature_md(ctx_verify,
                                                       classical_md) <= 0) ||
                        (EVP_PKEY_verify(ctx_verify, buf, buf_len, digest,
                                         digest_len) <= 0)) {
                        ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
                        OPENSSL_free(name);
                        CompositeSignature_free(compsig);
                        OPENSSL_free(final_tbs);
                        goto endverify;
                    }
                }
            }

            OPENSSL_free(name);
        }
        CompositeSignature_free(compsig);
        OPENSSL_free(final_tbs);
    } else {
        if (!oqsxkey->comp_pubkey[oqsxkey->numkeys - 1]) {
            ERR_raise(ERR_LIB_USER, OQSPROV_R_WRONG_PARAMETERS);
            goto endverify;
        }
#if !defined OQS_VERSION_MINOR ||                                              \
    (OQS_VERSION_MAJOR == 0 && OQS_VERSION_MINOR < 12)
        if (OQS_SIG_verify(
                oqs_key, tbs, tbslen, sig + index, siglen - classical_sig_len,
                oqsxkey->comp_pubkey[oqsxkey->numkeys - 1]) != OQS_SUCCESS) {
#else
        if (OQS_SIG_verify_with_ctx_str(
                oqs_key, tbs, tbslen, sig + index, siglen - classical_sig_len,
                poqs_sigctx->context_string, poqs_sigctx->context_string_length,
                oqsxkey->comp_pubkey[oqsxkey->numkeys - 1]) != OQS_SUCCESS) {
#endif
            ERR_raise(ERR_LIB_USER, OQSPROV_R_VERIFY_ERROR);
            goto endverify;
        }
    }
    rv = 1;

endverify:
    if (ctx_verify) {
        EVP_PKEY_CTX_free(ctx_verify);
    }
    OQS_SIG_PRINTF2("OQS SIG provider: verify rv = %d\n", rv);
    return rv;
}

static int oqs_sig_digest_signverify_init(void *vpoqs_sigctx,
                                          const char *mdname, void *voqssig,
                                          int operation) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    OQS_SIG_PRINTF2(
        "OQS SIG provider: digest_signverify_init called for mdname %s\n",
        mdname);

    poqs_sigctx->flag_allow_md = 1; /* permitted until first use */
    if (!oqs_sig_signverify_init(vpoqs_sigctx, voqssig, operation))
        return 0;

    if (!oqs_sig_setup_md(poqs_sigctx, mdname, NULL))
        return 0;

    if (mdname != NULL) {
        poqs_sigctx->mdctx = EVP_MD_CTX_new();
        if (poqs_sigctx->mdctx == NULL)
            goto error;

        if (!EVP_DigestInit_ex(poqs_sigctx->mdctx, poqs_sigctx->md, NULL))
            goto error;
    }

    return 1;

error:
    EVP_MD_CTX_free(poqs_sigctx->mdctx);
    EVP_MD_free(poqs_sigctx->md);
    poqs_sigctx->mdctx = NULL;
    poqs_sigctx->md = NULL;
    OQS_SIG_PRINTF("   OQS SIG provider: digest_signverify FAILED\n");
    return 0;
}

static int oqs_sig_digest_sign_init(void *vpoqs_sigctx, const char *mdname,
                                    void *voqssig, const OSSL_PARAM params[]) {
    OQS_SIG_PRINTF("OQS SIG provider: digest_sign_init called\n");
    return oqs_sig_digest_signverify_init(vpoqs_sigctx, mdname, voqssig,
                                          EVP_PKEY_OP_SIGN);
}

static int oqs_sig_digest_verify_init(void *vpoqs_sigctx, const char *mdname,
                                      void *voqssig,
                                      const OSSL_PARAM params[]) {
    OQS_SIG_PRINTF("OQS SIG provider: sig_digest_verify called\n");
    return oqs_sig_digest_signverify_init(vpoqs_sigctx, mdname, voqssig,
                                          EVP_PKEY_OP_VERIFY);
}

int oqs_sig_digest_signverify_update(void *vpoqs_sigctx,
                                     const unsigned char *data,
                                     size_t datalen) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    OQS_SIG_PRINTF("OQS SIG provider: digest_signverify_update called\n");

    if (poqs_sigctx == NULL)
        return 0;
    // disallow MD changes after update has been called at least once
    poqs_sigctx->flag_allow_md = 0;

    if (poqs_sigctx->mdctx)
        return EVP_DigestUpdate(poqs_sigctx->mdctx, data, datalen);
    else {
        // unconditionally collect data for passing in full to OQS API
        if (poqs_sigctx->mddata) {
            unsigned char *newdata = OPENSSL_realloc(
                poqs_sigctx->mddata, poqs_sigctx->mdsize + datalen);
            if (newdata == NULL)
                return 0;
            memcpy(newdata + poqs_sigctx->mdsize, data, datalen);
            poqs_sigctx->mddata = newdata;
            poqs_sigctx->mdsize += datalen;
        } else { // simple alloc and copy
            poqs_sigctx->mddata = OPENSSL_malloc(datalen);
            if (poqs_sigctx->mddata == NULL)
                return 0;
            poqs_sigctx->mdsize = datalen;
            memcpy(poqs_sigctx->mddata, data, poqs_sigctx->mdsize);
        }
        OQS_SIG_PRINTF2("OQS SIG provider: digest_signverify_update collected "
                        "%ld bytes...\n",
                        poqs_sigctx->mdsize);
    }
    return 1;
}

int oqs_sig_digest_sign_final(void *vpoqs_sigctx, unsigned char *sig,
                              size_t *siglen, size_t sigsize) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int dlen = 0;

    OQS_SIG_PRINTF("OQS SIG provider: digest_sign_final called\n");
    if (poqs_sigctx == NULL)
        return 0;

    /*
     * If sig is NULL then we're just finding out the sig size. Other fields
     * are ignored. Defer to oqs_sig_sign.
     */
    if (sig != NULL) {
        /*
         * TODO(3.0): There is the possibility that some externally
         * provided digests exceed EVP_MAX_MD_SIZE. We should probably
         * handle that somehow - but that problem is much larger than just
         * here.
         */
        if (poqs_sigctx->mdctx != NULL)
            if (!EVP_DigestFinal_ex(poqs_sigctx->mdctx, digest, &dlen))
                return 0;
    }

    poqs_sigctx->flag_allow_md = 1;

    if (poqs_sigctx->mdctx != NULL)
        return oqs_sig_sign(vpoqs_sigctx, sig, siglen, sigsize, digest,
                            (size_t)dlen);
    else
        return oqs_sig_sign(vpoqs_sigctx, sig, siglen, sigsize,
                            poqs_sigctx->mddata, poqs_sigctx->mdsize);
}

int oqs_sig_digest_verify_final(void *vpoqs_sigctx, const unsigned char *sig,
                                size_t siglen) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int dlen = 0;

    OQS_SIG_PRINTF("OQS SIG provider: digest_verify_final called\n");
    if (poqs_sigctx == NULL)
        return 0;

    // TBC for hybrids:
    if (poqs_sigctx->mdctx) {
        if (!EVP_DigestFinal_ex(poqs_sigctx->mdctx, digest, &dlen))
            return 0;

        poqs_sigctx->flag_allow_md = 1;

        return oqs_sig_verify(vpoqs_sigctx, sig, siglen, digest, (size_t)dlen);
    } else
        return oqs_sig_verify(vpoqs_sigctx, sig, siglen, poqs_sigctx->mddata,
                              poqs_sigctx->mdsize);
}

static void oqs_sig_freectx(void *vpoqs_sigctx) {
    PROV_OQSSIG_CTX *ctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    OQS_SIG_PRINTF("OQS SIG provider: freectx called\n");
    OPENSSL_free(ctx->propq);
    EVP_MD_CTX_free(ctx->mdctx);
    EVP_MD_free(ctx->md);
    ctx->propq = NULL;
    ctx->mdctx = NULL;
    ctx->md = NULL;
    oqsx_key_free(ctx->sig);
    OPENSSL_free(ctx->mddata);
    ctx->mddata = NULL;
    ctx->mdsize = 0;
    OPENSSL_free(ctx->aid);
    ctx->aid = NULL;
    ctx->aid_len = 0;
    OPENSSL_free(ctx->context_string);
    ctx->context_string = NULL;
    ctx->context_string_length = 0;
    OPENSSL_free(ctx);
}

static void *oqs_sig_dupctx(void *vpoqs_sigctx) {
    PROV_OQSSIG_CTX *srcctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;
    PROV_OQSSIG_CTX *dstctx;

    OQS_SIG_PRINTF("OQS SIG provider: dupctx called\n");

    dstctx = OPENSSL_zalloc(sizeof(*srcctx));
    if (dstctx == NULL)
        return NULL;

    *dstctx = *srcctx;
    dstctx->sig = NULL;
    dstctx->md = NULL;
    dstctx->mdctx = NULL;

    if (srcctx->sig != NULL && !oqsx_key_up_ref(srcctx->sig))
        goto err;
    dstctx->sig = srcctx->sig;

    if (srcctx->md != NULL && !EVP_MD_up_ref(srcctx->md))
        goto err;
    dstctx->md = srcctx->md;

    if (srcctx->mdctx != NULL) {
        dstctx->mdctx = EVP_MD_CTX_new();
        if (dstctx->mdctx == NULL ||
            !EVP_MD_CTX_copy_ex(dstctx->mdctx, srcctx->mdctx))
            goto err;
    }

    if (srcctx->mddata) {
        dstctx->mddata = OPENSSL_memdup(srcctx->mddata, srcctx->mdsize);
        if (dstctx->mddata == NULL)
            goto err;
        dstctx->mdsize = srcctx->mdsize;
    }

    if (srcctx->aid) {
        dstctx->aid = OPENSSL_memdup(srcctx->aid, srcctx->aid_len);
        if (dstctx->aid == NULL)
            goto err;
        dstctx->aid_len = srcctx->aid_len;
    }

    if (srcctx->propq) {
        dstctx->propq = OPENSSL_strdup(srcctx->propq);
        if (dstctx->propq == NULL)
            goto err;
    }

    return dstctx;
err:
    oqs_sig_freectx(dstctx);
    return NULL;
}

static int oqs_sig_get_ctx_params(void *vpoqs_sigctx, OSSL_PARAM *params) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;
    OSSL_PARAM *p;

    OQS_SIG_PRINTF("OQS SIG provider: get_ctx_params called\n");
    if (poqs_sigctx == NULL || params == NULL)
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_SIGNATURE_PARAM_ALGORITHM_ID);

    if (poqs_sigctx->aid == NULL) {
        poqs_sigctx->aid_len =
            get_aid(&(poqs_sigctx->aid), poqs_sigctx->sig->tls_name);
    }

    if (p != NULL &&
        !OSSL_PARAM_set_octet_string(p, poqs_sigctx->aid, poqs_sigctx->aid_len))
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_SIGNATURE_PARAM_DIGEST);
    if (p != NULL && !OSSL_PARAM_set_utf8_string(p, poqs_sigctx->mdname))
        return 0;

    return 1;
}

static const OSSL_PARAM known_gettable_ctx_params[] = {
    OSSL_PARAM_octet_string(OSSL_SIGNATURE_PARAM_ALGORITHM_ID, NULL, 0),
    OSSL_PARAM_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST, NULL, 0),
    OSSL_PARAM_END};

static const OSSL_PARAM *
oqs_sig_gettable_ctx_params(ossl_unused void *vpoqs_sigctx,
                            ossl_unused void *vctx) {
    OQS_SIG_PRINTF("OQS SIG provider: gettable_ctx_params called\n");
    return known_gettable_ctx_params;
}
static int oqs_sig_set_ctx_params(void *vpoqs_sigctx,
                                  const OSSL_PARAM params[]) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;
    const OSSL_PARAM *p;

    OQS_SIG_PRINTF("OQS SIG provider: set_ctx_params called\n");
    if (poqs_sigctx == NULL || params == NULL)
        return 0;

    p = OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_DIGEST);
    /* Not allowed during certain operations */
    if (p != NULL && !poqs_sigctx->flag_allow_md)
        return 0;
    if (p != NULL) {
        char mdname[OSSL_MAX_NAME_SIZE] = "", *pmdname = mdname;
        char mdprops[OSSL_MAX_PROPQUERY_SIZE] = "", *pmdprops = mdprops;
        const OSSL_PARAM *propsp =
            OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_PROPERTIES);

        if (!OSSL_PARAM_get_utf8_string(p, &pmdname, sizeof(mdname)))
            return 0;
        if (propsp != NULL &&
            !OSSL_PARAM_get_utf8_string(propsp, &pmdprops, sizeof(mdprops)))
            return 0;
        if (!oqs_sig_setup_md(poqs_sigctx, mdname, mdprops))
            return 0;
    }
#if (OPENSSL_VERSION_PREREQ(3, 2))
    p = OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_CONTEXT_STRING);
    if (p != NULL) {
        if (!OSSL_PARAM_get_octet_string(p, &poqs_sigctx->context_string, 0,
                                         &poqs_sigctx->context_string_length)) {
            poqs_sigctx->context_string_length = 0;
            return 0;
        }
    }
#endif

    // not passing in parameters we can act on is no error
    return 1;
}

static const OSSL_PARAM known_settable_ctx_params[] = {
    OSSL_PARAM_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST, NULL, 0),
    OSSL_PARAM_utf8_string(OSSL_SIGNATURE_PARAM_PROPERTIES, NULL, 0),
#if (OPENSSL_VERSION_PREREQ(3, 2))
    OSSL_PARAM_octet_string(OSSL_SIGNATURE_PARAM_CONTEXT_STRING, NULL, 0),
#endif
    OSSL_PARAM_END};

static const OSSL_PARAM *
oqs_sig_settable_ctx_params(ossl_unused void *vpsm2ctx,
                            ossl_unused void *provctx) {
    /*
     * TODO(3.0): Should this function return a different set of settable ctx
     * params if the ctx is being used for a DigestSign/DigestVerify? In that
     * case it is not allowed to set the digest size/digest name because the
     * digest is explicitly set as part of the init.
     * NOTE: Ideally we would check poqs_sigctx->flag_allow_md, but this is
     * problematic because there is no nice way of passing the
     * PROV_OQSSIG_CTX down to this function...
     * Because we have API's that dont know about their parent..
     * e.g: EVP_SIGNATURE_gettable_ctx_params(const EVP_SIGNATURE *sig).
     * We could pass NULL for that case (but then how useful is the check?).
     */
    OQS_SIG_PRINTF("OQS SIG provider: settable_ctx_params called\n");
    return known_settable_ctx_params;
}

static int oqs_sig_get_ctx_md_params(void *vpoqs_sigctx, OSSL_PARAM *params) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    OQS_SIG_PRINTF("OQS SIG provider: get_ctx_md_params called\n");
    if (poqs_sigctx->mdctx == NULL)
        return 0;

    return EVP_MD_CTX_get_params(poqs_sigctx->mdctx, params);
}

static const OSSL_PARAM *oqs_sig_gettable_ctx_md_params(void *vpoqs_sigctx) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    OQS_SIG_PRINTF("OQS SIG provider: gettable_ctx_md_params called\n");
    if (poqs_sigctx->md == NULL)
        return 0;

    return EVP_MD_gettable_ctx_params(poqs_sigctx->md);
}

static int oqs_sig_set_ctx_md_params(void *vpoqs_sigctx,
                                     const OSSL_PARAM params[]) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    OQS_SIG_PRINTF("OQS SIG provider: set_ctx_md_params called\n");
    if (poqs_sigctx->mdctx == NULL)
        return 0;

    return EVP_MD_CTX_set_params(poqs_sigctx->mdctx, params);
}

static const OSSL_PARAM *oqs_sig_settable_ctx_md_params(void *vpoqs_sigctx) {
    PROV_OQSSIG_CTX *poqs_sigctx = (PROV_OQSSIG_CTX *)vpoqs_sigctx;

    if (poqs_sigctx->md == NULL)
        return 0;

    OQS_SIG_PRINTF("OQS SIG provider: settable_ctx_md_params called\n");
    return EVP_MD_settable_ctx_params(poqs_sigctx->md);
}

const OSSL_DISPATCH oqs_signature_functions[] = {
    {OSSL_FUNC_SIGNATURE_NEWCTX, (void (*)(void))oqs_sig_newctx},
    {OSSL_FUNC_SIGNATURE_SIGN_INIT, (void (*)(void))oqs_sig_sign_init},
    {OSSL_FUNC_SIGNATURE_SIGN, (void (*)(void))oqs_sig_sign},
    {OSSL_FUNC_SIGNATURE_VERIFY_INIT, (void (*)(void))oqs_sig_verify_init},
    {OSSL_FUNC_SIGNATURE_VERIFY, (void (*)(void))oqs_sig_verify},
    {OSSL_FUNC_SIGNATURE_DIGEST_SIGN_INIT,
     (void (*)(void))oqs_sig_digest_sign_init},
    {OSSL_FUNC_SIGNATURE_DIGEST_SIGN_UPDATE,
     (void (*)(void))oqs_sig_digest_signverify_update},
    {OSSL_FUNC_SIGNATURE_DIGEST_SIGN_FINAL,
     (void (*)(void))oqs_sig_digest_sign_final},
    {OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_INIT,
     (void (*)(void))oqs_sig_digest_verify_init},
    {OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_UPDATE,
     (void (*)(void))oqs_sig_digest_signverify_update},
    {OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_FINAL,
     (void (*)(void))oqs_sig_digest_verify_final},
    {OSSL_FUNC_SIGNATURE_FREECTX, (void (*)(void))oqs_sig_freectx},
    {OSSL_FUNC_SIGNATURE_DUPCTX, (void (*)(void))oqs_sig_dupctx},
    {OSSL_FUNC_SIGNATURE_GET_CTX_PARAMS,
     (void (*)(void))oqs_sig_get_ctx_params},
    {OSSL_FUNC_SIGNATURE_GETTABLE_CTX_PARAMS,
     (void (*)(void))oqs_sig_gettable_ctx_params},
    {OSSL_FUNC_SIGNATURE_SET_CTX_PARAMS,
     (void (*)(void))oqs_sig_set_ctx_params},
    {OSSL_FUNC_SIGNATURE_SETTABLE_CTX_PARAMS,
     (void (*)(void))oqs_sig_settable_ctx_params},
    {OSSL_FUNC_SIGNATURE_GET_CTX_MD_PARAMS,
     (void (*)(void))oqs_sig_get_ctx_md_params},
    {OSSL_FUNC_SIGNATURE_GETTABLE_CTX_MD_PARAMS,
     (void (*)(void))oqs_sig_gettable_ctx_md_params},
    {OSSL_FUNC_SIGNATURE_SET_CTX_MD_PARAMS,
     (void (*)(void))oqs_sig_set_ctx_md_params},
    {OSSL_FUNC_SIGNATURE_SETTABLE_CTX_MD_PARAMS,
     (void (*)(void))oqs_sig_settable_ctx_md_params},
    {0, NULL}};
