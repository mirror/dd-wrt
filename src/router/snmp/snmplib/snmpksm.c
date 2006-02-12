/*
 * snmpksm.c
 *
 * This code implements the Kerberos Security Model (KSM) for SNMP.
 *
 * Security number - 2066432
 */

#include <net-snmp/net-snmp-config.h>

#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <errno.h>


#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <krb5.h>
#include <com_err.h>

#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/asn1.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/keytools.h>
#include <net-snmp/library/snmpv3.h>
#include <net-snmp/library/lcd_time.h>
#include <net-snmp/library/scapi.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/snmpksm.h>

static krb5_context kcontext = NULL;
static krb5_rcache rcache = NULL;

static int      ksm_session_init(netsnmp_session *);
static void     ksm_free_state_ref(void *);
static int      ksm_free_pdu(netsnmp_pdu *);
static int      ksm_clone_pdu(netsnmp_pdu *, netsnmp_pdu *);

static int      ksm_insert_cache(long, krb5_auth_context, u_char *,
                                 size_t);
static void     ksm_decrement_ref_count(long);
static void     ksm_increment_ref_count(long);
static struct ksm_cache_entry *ksm_get_cache(long);

#define HASHSIZE	64

/*
 * Our information stored for the response PDU.
 */

struct ksm_secStateRef {
    krb5_auth_context auth_context;
    krb5_cksumtype  cksumtype;
};

/*
 * A KSM outgoing pdu cache entry
 */

struct ksm_cache_entry {
    long            msgid;
    int             refcount;
    krb5_auth_context auth_context;
    u_char         *secName;
    size_t          secNameLen;
    struct ksm_cache_entry *next;
};

/*
 * Poor man's hash table
 */

static struct ksm_cache_entry *ksm_hash_table[HASHSIZE];

/*
 * Initialize all of the state required for Kerberos (right now, just call
 * krb5_init_context).
 */

void
init_ksm(void)
{
    krb5_error_code retval;
    struct snmp_secmod_def *def;
    int             i;

    if (kcontext == NULL) {
        retval = krb5_init_context(&kcontext);

        if (retval) {
            DEBUGMSGTL(("ksm", "krb5_init_context failed (%s), not "
                        "registering KSM\n", error_message(retval)));
            return;
        }
    }

    for (i = 0; i < HASHSIZE; i++)
        ksm_hash_table[i] = NULL;

    def = SNMP_MALLOC_STRUCT(snmp_secmod_def);

    if (!def) {
        DEBUGMSGTL(("ksm", "Unable to malloc snmp_secmod struct, not "
                    "registering KSM\n"));
        return;
    }

    def->encode_reverse = ksm_rgenerate_out_msg;
    def->decode = ksm_process_in_msg;
    def->session_open = ksm_session_init;
    def->pdu_free_state_ref = ksm_free_state_ref;
    def->pdu_free = ksm_free_pdu;
    def->pdu_clone = ksm_clone_pdu;

    register_sec_mod(2066432, "ksm", def);
}

/*
 * These routines implement a simple cache for information we need to
 * process responses.  When we send out a request, it contains an AP_REQ;
 * we get back an AP_REP, and we need the authorization context from the
 * AP_REQ to decrypt the AP_REP.  But because right now there's nothing
 * that gets preserved across calls to rgenerate_out_msg to process_in_msg,
 * we cache these internally based on the message ID (we also cache the
 * passed-in security name, for reasons that are mostly stupid).
 */

static int
ksm_insert_cache(long msgid, krb5_auth_context auth_context,
                 u_char * secName, size_t secNameLen)
{
    struct ksm_cache_entry *entry;
    int             bucket;
    int             retval;

    entry = SNMP_MALLOC_STRUCT(ksm_cache_entry);

    if (!entry)
        return SNMPERR_MALLOC;

    entry->msgid = msgid;
    entry->auth_context = auth_context;
    entry->refcount = 1;

    retval = memdup(&entry->secName, secName, secNameLen);

    if (retval != SNMPERR_SUCCESS) {
        free(entry);
        return retval;
    }

    entry->secNameLen = secNameLen;

    bucket = msgid % HASHSIZE;

    entry->next = ksm_hash_table[bucket];
    ksm_hash_table[bucket] = entry;

    return SNMPERR_SUCCESS;
}

static struct ksm_cache_entry *
ksm_get_cache(long msgid)
{
    struct ksm_cache_entry *entry;
    int             bucket;

    bucket = msgid % HASHSIZE;

    for (entry = ksm_hash_table[bucket]; entry != NULL;
         entry = entry->next)
        if (entry->msgid == msgid)
            return entry;

    return NULL;
}

static void
ksm_decrement_ref_count(long msgid)
{
    struct ksm_cache_entry *entry, *entry1;
    int             bucket;

    bucket = msgid % HASHSIZE;

    if (ksm_hash_table[bucket] && ksm_hash_table[bucket]->msgid == msgid) {
        entry = ksm_hash_table[bucket];

        /*
         * If the reference count is zero, then free it
         */

        if (--entry->refcount <= 0) {
            DEBUGMSGTL(("ksm", "Freeing entry for msgid %ld\n", msgid));
            krb5_auth_con_free(kcontext, entry->auth_context);
            free(entry->secName);
            ksm_hash_table[bucket] = entry->next;
            free(entry);
        }

        return;

    } else if (ksm_hash_table[bucket])
        for (entry1 = ksm_hash_table[bucket], entry = entry1->next;
             entry != NULL; entry1 = entry, entry = entry->next)
            if (entry->msgid == msgid) {

                if (--entry->refcount <= 0) {
                    DEBUGMSGTL(("ksm", "Freeing entry for msgid %ld\n",
                                msgid));
                    krb5_auth_con_free(kcontext, entry->auth_context);
                    free(entry->secName);
                    entry1->next = entry->next;
                    free(entry);
                }

                return;
            }

    DEBUGMSGTL(("ksm",
                "KSM: Unable to decrement cache entry for msgid %ld.\n",
                msgid));
}

static void
ksm_increment_ref_count(long msgid)
{
    struct ksm_cache_entry *entry = ksm_get_cache(msgid);

    if (!entry) {
        DEBUGMSGTL(("ksm", "Unable to find cache entry for msgid %ld "
                    "for increment\n", msgid));
        return;
    }

    entry->refcount++;
}

/*
 * Initialize specific session information (right now, just set up things to
 * not do an engineID probe)
 */

static int
ksm_session_init(netsnmp_session * sess)
{
    DEBUGMSGTL(("ksm",
                "KSM: Reached our session initialization callback\n"));

    sess->flags |= SNMP_FLAGS_DONT_PROBE;

    return SNMPERR_SUCCESS;
}

/*
 * Free our state information (this is only done on the agent side)
 */

static void
ksm_free_state_ref(void *ptr)
{
    struct ksm_secStateRef *ref = (struct ksm_secStateRef *) ptr;

    DEBUGMSGTL(("ksm", "KSM: Freeing state reference\n"));

    krb5_auth_con_free(kcontext, ref->auth_context);

    free(ref);
}

/*
 * This is called when the PDU is freed; this will decrement reference counts
 * for entries in our state cache.
 */

static int
ksm_free_pdu(netsnmp_pdu *pdu)
{
    ksm_decrement_ref_count(pdu->msgid);

    DEBUGMSGTL(("ksm", "Decrementing cache entry for PDU msgid %ld\n",
                pdu->msgid));

    return SNMPERR_SUCCESS;
}

/*
 * This is called when a PDU is cloned (to increase reference counts)
 */

static int
ksm_clone_pdu(netsnmp_pdu *pdu, netsnmp_pdu *pdu2)
{
    ksm_increment_ref_count(pdu->msgid);

    DEBUGMSGTL(("ksm", "Incrementing cache entry for PDU msgid %ld\n",
                pdu->msgid));

    return SNMPERR_SUCCESS;
}

/****************************************************************************
 *
 * ksm_generate_out_msg
 *
 * Parameters:
 *	(See list below...)
 *
 * Returns:
 *	SNMPERR_GENERIC                        On success.
 *	SNMPERR_KRB5
 *	... and others
 *
 *
 * Generate an outgoing message.
 *
 ****************************************************************************/

int
ksm_rgenerate_out_msg(struct snmp_secmod_outgoing_params *parms)
{
    krb5_auth_context auth_context = NULL;
    krb5_error_code retcode;
    krb5_ccache     cc = NULL;
    int             retval = SNMPERR_SUCCESS;
    krb5_data       outdata, ivector;
    krb5_keyblock  *subkey = NULL;
#ifdef MIT_NEW_CRYPTO
    krb5_data       input;
    krb5_enc_data   output;
    unsigned int    numcksumtypes;
    krb5_cksumtype  *cksumtype_array;
#else                           /* MIT_NEW_CRYPTO */
    krb5_encrypt_block eblock;
#endif                          /* MIT_NEW_CRYPTO */
    size_t          blocksize, encrypted_length;
    unsigned char  *encrypted_data = NULL;
    int             zero = 0, i;
    u_char         *cksum_pointer, *endp = *parms->wholeMsg;
    krb5_cksumtype  cksumtype;
    krb5_checksum   pdu_checksum;
    u_char         **wholeMsg = parms->wholeMsg;
    size_t	   *offset = parms->wholeMsgOffset, seq_offset;
    struct ksm_secStateRef *ksm_state = (struct ksm_secStateRef *)
        parms->secStateRef;
    int rc;

    DEBUGMSGTL(("ksm", "Starting KSM processing\n"));

    outdata.length = 0;
    outdata.data = NULL;
    ivector.length = 0;
    ivector.data = NULL;
    pdu_checksum.contents = NULL;

    if (!ksm_state) {
        /*
         * If we don't have a ksm_state, then we're a request.  Get a
         * credential cache and build a ap_req.
         */
        retcode = krb5_cc_default(kcontext, &cc);

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM: krb5_cc_default failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }

        DEBUGMSGTL(("ksm", "KSM: Set credential cache successfully\n"));

        /*
         * This seems odd, since we don't need this until later (or earlier,
         * depending on how you look at it), but because the most likely
         * errors are Kerberos at this point, I'll get this now to save
         * time not encoding the rest of the packet.
         *
         * Also, we need the subkey to encrypt the PDU (if required).
         */

        retcode =
            krb5_mk_req(kcontext, &auth_context,
                        AP_OPTS_MUTUAL_REQUIRED | AP_OPTS_USE_SUBKEY,
                        (char *) "host", parms->session->peername, NULL,
                        cc, &outdata);

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM: krb5_mk_req failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }

        DEBUGMSGTL(("ksm", "KSM: ticket retrieved successfully\n"));

    } else {

        /*
         * Grab the auth_context from our security state reference
         */

        auth_context = ksm_state->auth_context;

        /*
         * Bundle up an AP_REP.  Note that we do this only when we
         * have a security state reference (which means we're in an agent
         * and we're sending a response).
         */

        DEBUGMSGTL(("ksm", "KSM: Starting reply processing.\n"));

        retcode = krb5_mk_rep(kcontext, auth_context, &outdata);

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM: krb5_mk_rep failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }

        DEBUGMSGTL(("ksm", "KSM: Finished with krb5_mk_rep()\n"));
    }

    /*
     * If we have to encrypt the PDU, do that now
     */

    if (parms->secLevel == SNMP_SEC_LEVEL_AUTHPRIV) {

        DEBUGMSGTL(("ksm", "KSM: Starting PDU encryption.\n"));

        /*
         * It's weird -
         *
         * If we're on the manager, it's a local subkey (because that's in
         * our AP_REQ)
         *
         * If we're on the agent, it's a remote subkey (because that comes
         * FROM the received AP_REQ).
         */

        if (ksm_state)
            retcode = krb5_auth_con_getremotesubkey(kcontext, auth_context,
                                                    &subkey);
        else
            retcode = krb5_auth_con_getlocalsubkey(kcontext, auth_context,
                                                   &subkey);

        if (retcode) {
            DEBUGMSGTL(("ksm",
                        "KSM: krb5_auth_con_getlocalsubkey failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }

        /*
         * Note that here we need to handle different things between the
         * old and new crypto APIs.  First, we need to get the final encrypted
         * length of the PDU.
         */

#ifdef MIT_NEW_CRYPTO
        retcode = krb5_c_encrypt_length(kcontext, subkey->enctype,
                                        parms->scopedPduLen,
                                        &encrypted_length);

        if (retcode) {
            DEBUGMSGTL(("ksm",
                        "Encryption length calculation failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }
#else                           /* MIT_NEW_CRYPTO */

        krb5_use_enctype(kcontext, &eblock, subkey->enctype);
        retcode = krb5_process_key(kcontext, &eblock, subkey);

        if (retcode) {
            DEBUGMSGTL(("ksm", "krb5_process_key failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }

        encrypted_length = krb5_encrypt_size(parms->scopedPduLen,
                                             eblock.crypto_entry);
#endif                          /* MIT_NEW_CRYPTO */

        encrypted_data = malloc(encrypted_length);

        if (!encrypted_data) {
            DEBUGMSGTL(("ksm",
                        "KSM: Unable to malloc %d bytes for encrypt "
                        "buffer: %s\n", parms->scopedPduLen,
                        strerror(errno)));
            retval = SNMPERR_MALLOC;
#ifndef MIT_NEW_CRYPTO
            krb5_finish_key(kcontext, &eblock);
#endif                          /* ! MIT_NEW_CRYPTO */

            goto error;
        }

        /*
         * We need to set up a blank initialization vector for the encryption.
         * Use a block of all zero's (which is dependent on the block size
         * of the encryption method).
         */

#ifdef MIT_NEW_CRYPTO

        retcode = krb5_c_block_size(kcontext, subkey->enctype, &blocksize);

        if (retcode) {
            DEBUGMSGTL(("ksm",
                        "Unable to determine crypto block size: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }
#else                           /* MIT_NEW_CRYPTO */

        blocksize =
            krb5_enctype_array[subkey->enctype]->system->block_length;

#endif                          /* MIT_NEW_CRYPTO */

        ivector.data = malloc(blocksize);

        if (!ivector.data) {
            DEBUGMSGTL(("ksm", "Unable to allocate %d bytes for ivector\n",
                        blocksize));
            retval = SNMPERR_MALLOC;
            goto error;
        }

        ivector.length = blocksize;
        memset(ivector.data, 0, blocksize);

        /*
         * Finally!  Do the encryption!
         */

#ifdef MIT_NEW_CRYPTO

        input.data = (char *) parms->scopedPdu;
        input.length = parms->scopedPduLen;
        output.ciphertext.data = (char *) encrypted_data;
        output.ciphertext.length = encrypted_length;

        retcode =
            krb5_c_encrypt(kcontext, subkey, KSM_KEY_USAGE_ENCRYPTION,
                           &ivector, &input, &output);

#else                           /* MIT_NEW_CRYPTO */

        retcode = krb5_encrypt(kcontext, (krb5_pointer) parms->scopedPdu,
                               (krb5_pointer) encrypted_data,
                               parms->scopedPduLen, &eblock, ivector.data);

        krb5_finish_key(kcontext, &eblock);

#endif                          /* MIT_NEW_CRYPTO */

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM: krb5_encrypt failed: %s\n",
                        error_message(retcode)));
            retval = SNMPERR_KRB5;
            snmp_set_detail(error_message(retcode));
            goto error;
        }

	*offset = 0;

        rc = asn_realloc_rbuild_string(wholeMsg, parms->wholeMsgLen,
                                             offset, 1,
                                             (u_char) (ASN_UNIVERSAL |
                                                       ASN_PRIMITIVE |
                                                       ASN_OCTET_STR),
                                             encrypted_data,
                                             encrypted_length);

        if (rc == 0) {
            DEBUGMSGTL(("ksm", "Building encrypted payload failed.\n"));
            retval = SNMPERR_TOO_LONG;
            goto error;
        }

        DEBUGMSGTL(("ksm", "KSM: Encryption complete.\n"));

    } else {
        /*
         * Plaintext PDU (not encrypted)
         */

        if (*parms->wholeMsgLen < parms->scopedPduLen) {
            DEBUGMSGTL(("ksm", "Not enough room for plaintext PDU.\n"));
            retval = SNMPERR_TOO_LONG;
            goto error;
        }
    }

    /*
     * Start encoding the msgSecurityParameters
     *
     * For now, use 0 for the response hint
     */

    DEBUGMSGTL(("ksm", "KSM: scopedPdu added to payload\n"));

    seq_offset = *offset;

    rc = asn_realloc_rbuild_int(wholeMsg, parms->wholeMsgLen,
                                      offset, 1,
                                      (u_char) (ASN_UNIVERSAL |
                                                ASN_PRIMITIVE |
                                                ASN_INTEGER),
                                      (long *) &zero, sizeof(zero));

    if (rc == 0) {
        DEBUGMSGTL(("ksm", "Building ksm security parameters failed.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    rc = asn_realloc_rbuild_string(wholeMsg, parms->wholeMsgLen,
                                         offset, 1,
                                         (u_char) (ASN_UNIVERSAL |
                                                   ASN_PRIMITIVE |
                                                   ASN_OCTET_STR),
                                         (u_char *) outdata.data,
                                         outdata.length);

    if (rc == 0) {
        DEBUGMSGTL(("ksm", "Building ksm AP_REQ failed.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    /*
     * Now, we need to pick the "right" checksum algorithm.  For old
     * crypto, just pick CKSUMTYPE_RSA_MD5_DES; for new crypto, pick
     * one of the "approved" ones.
     */

#ifdef MIT_NEW_CRYPTO
    retcode = krb5_c_keyed_checksum_types(kcontext, subkey->enctype,
                                          &numcksumtypes, &cksumtype_array);

    if (retcode) {
	DEBUGMSGTL(("ksm", "Unable to find appropriate keyed checksum: %s\n",
		    error_message(retcode)));
	snmp_set_detail(error_message(retcode));
        retval = SNMPERR_KRB5;
        goto error;
    }

    if (numcksumtypes <= 0) {
	DEBUGMSGTL(("ksm", "We received a list of zero cksumtypes for this "
		    "enctype (%d)\n", subkey->enctype));
	snmp_set_detail("No valid checksum type for this encryption type");
	retval = SNMPERR_KRB5;
	goto error;
    }

    /*
     * It's not clear to me from the API which checksum you're supposed
     * to support, so I'm taking a guess at the first one
     */

    cksumtype = cksumtype_array[0];

    krb5_free_cksumtypes(kcontext, cksumtype_array);

    DEBUGMSGTL(("ksm", "KSM: Choosing checksum type of %d (subkey type "
		"of %d)\n", cksumtype, subkey->enctype));

    retcode = krb5_c_checksum_length(kcontext, cksumtype, &blocksize);

    if (retcode) {
        DEBUGMSGTL(("ksm", "Unable to determine checksum length: %s\n",
                    error_message(retcode)));
        snmp_set_detail(error_message(retcode));
        retval = SNMPERR_KRB5;
        goto error;
    }

    pdu_checksum.length = blocksize;

#else /* MIT_NEW_CRYPTO */
    if (ksm_state)
        cksumtype = ksm_state->cksumtype;
    else
	cksumtype = CKSUMTYPE_RSA_MD5_DES;

    if (!is_keyed_cksum(cksumtype)) {
        DEBUGMSGTL(("ksm", "Checksum type %d is not a keyed checksum\n",
                    cksumtype));
        snmp_set_detail("Checksum is not a keyed checksum");
        retval = SNMPERR_KRB5;
        goto error;
    }

    if (!is_coll_proof_cksum(cksumtype)) {
        DEBUGMSGTL(("ksm", "Checksum type %d is not a collision-proof "
                    "checksum\n", cksumtype));
        snmp_set_detail("Checksum is not a collision-proof checksum");
        retval = SNMPERR_KRB5;
        goto error;
    }

    pdu_checksum.length = krb5_checksum_size(kcontext, cksumtype);
    pdu_checksum.checksum_type = cksumtype;

#endif /* MIT_NEW_CRYPTO */

    /*
     * Note that here, we're just leaving blank space for the checksum;
     * we remember where that is, and we'll fill it in later.
     */

    *offset += pdu_checksum.length;
    memset(*wholeMsg + *parms->wholeMsgLen - *offset, 0, pdu_checksum.length);

    cksum_pointer = *wholeMsg + *parms->wholeMsgLen - *offset;

    rc = asn_realloc_rbuild_header(wholeMsg, parms->wholeMsgLen,
                                         parms->wholeMsgOffset, 1,
                                         (u_char) (ASN_UNIVERSAL |
                                                   ASN_PRIMITIVE |
                                                   ASN_OCTET_STR),
                                         pdu_checksum.length);

    if (rc == 0) {
        DEBUGMSGTL(("ksm", "Building ksm security parameters failed.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    rc = asn_realloc_rbuild_int(wholeMsg, parms->wholeMsgLen,
                                      parms->wholeMsgOffset, 1,
                                      (u_char) (ASN_UNIVERSAL |
                                                ASN_PRIMITIVE |
                                                ASN_OCTET_STR),
                                      (long *) &cksumtype,
                                      sizeof(cksumtype));

    if (rc == 0) {
        DEBUGMSGTL(("ksm", "Building ksm security parameters failed.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    rc = asn_realloc_rbuild_sequence(wholeMsg, parms->wholeMsgLen,
                                           parms->wholeMsgOffset, 1,
                                           (u_char) (ASN_SEQUENCE |
                                                     ASN_CONSTRUCTOR),
                                           *offset - seq_offset);

    if (rc == 0) {
        DEBUGMSGTL(("ksm", "Building ksm security parameters failed.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    rc = asn_realloc_rbuild_header(wholeMsg, parms->wholeMsgLen,
                                         parms->wholeMsgOffset, 1,
                                         (u_char) (ASN_UNIVERSAL |
                                                   ASN_PRIMITIVE |
                                                   ASN_OCTET_STR),
                                         *offset - seq_offset);

    if (rc == 0) {
        DEBUGMSGTL(("ksm", "Building ksm security parameters failed.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    DEBUGMSGTL(("ksm", "KSM: Security parameter encoding completed\n"));

    /*
     * We're done with the KSM security parameters - now we do the global
     * header and wrap up the whole PDU.
     */

    if (*parms->wholeMsgLen < parms->globalDataLen) {
        DEBUGMSGTL(("ksm", "Building global data failed.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    *offset += parms->globalDataLen;
    memcpy(*wholeMsg + *parms->wholeMsgLen - *offset,
	   parms->globalData, parms->globalDataLen);

    rc = asn_realloc_rbuild_sequence(wholeMsg, parms->wholeMsgLen,
                                           offset, 1,
                                           (u_char) (ASN_SEQUENCE |
                                                     ASN_CONSTRUCTOR),
                                           *offset);

    if (rc == 0) {
        DEBUGMSGTL(("ksm", "Building master packet sequence.\n"));
        retval = SNMPERR_TOO_LONG;
        goto error;
    }

    DEBUGMSGTL(("ksm", "KSM: PDU master packet encoding complete.\n"));

    /*
     * Now we need to checksum the entire PDU (since it's built).
     */

    pdu_checksum.contents = malloc(pdu_checksum.length);

    if (!pdu_checksum.contents) {
        DEBUGMSGTL(("ksm", "Unable to malloc %d bytes for checksum\n",
                    pdu_checksum.length));
        retval = SNMPERR_MALLOC;
        goto error;
    }

    /*
     * If we didn't encrypt the packet, we haven't yet got the subkey.
     * Get that now.
     */

    if (!subkey) {
        if (ksm_state)
            retcode = krb5_auth_con_getremotesubkey(kcontext, auth_context,
                                                    &subkey);
        else
            retcode = krb5_auth_con_getlocalsubkey(kcontext, auth_context,
                                                   &subkey);
        if (retcode) {
            DEBUGMSGTL(("ksm", "krb5_auth_con_getlocalsubkey failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }
    }
#ifdef MIT_NEW_CRYPTO

    input.data = (char *) (*wholeMsg + *parms->wholeMsgLen - *offset);
    input.length = *offset;
        retcode = krb5_c_make_checksum(kcontext, cksumtype, subkey,
                                       KSM_KEY_USAGE_CHECKSUM, &input,
                                       &pdu_checksum);

#else                           /* MIT_NEW_CRYPTO */

    retcode = krb5_calculate_checksum(kcontext, cksumtype, *wholeMsg +
				      *parms->wholeMsgLen - *offset,
                                      *offset,
                                      (krb5_pointer) subkey->contents,
                                      subkey->length, &pdu_checksum);

#endif                          /* MIT_NEW_CRYPTO */

    if (retcode) {
        DEBUGMSGTL(("ksm", "Calculate checksum failed: %s\n",
                    error_message(retcode)));
        retval = SNMPERR_KRB5;
        snmp_set_detail(error_message(retcode));
        goto error;
    }

    DEBUGMSGTL(("ksm", "KSM: Checksum calculation complete.\n"));

    memcpy(cksum_pointer, pdu_checksum.contents, pdu_checksum.length);

    DEBUGMSGTL(("ksm", "KSM: Writing checksum of %d bytes at offset %d\n",
                pdu_checksum.length, cksum_pointer - (*wholeMsg + 1)));

    DEBUGMSGTL(("ksm", "KSM: Checksum:"));

    for (i = 0; i < pdu_checksum.length; i++)
        DEBUGMSG(("ksm", " %02x",
                  (unsigned int) pdu_checksum.contents[i]));

    DEBUGMSG(("ksm", "\n"));

    /*
     * If we're _not_ called as part of a response (null ksm_state),
     * then save the auth_context for later using our cache routines.
     */

    if (!ksm_state) {
        if ((retval = ksm_insert_cache(parms->pdu->msgid, auth_context,
                                       (u_char *) parms->secName,
                                       parms->secNameLen)) !=
            SNMPERR_SUCCESS)
            goto error;
        auth_context = NULL;
    }

    DEBUGMSGTL(("ksm", "KSM processing complete!\n"));

  error:

    if (pdu_checksum.contents)
#ifdef MIT_NEW_CRYPTO
        krb5_free_checksum_contents(kcontext, &pdu_checksum);
#else                           /* MIT_NEW_CRYPTO */
        free(pdu_checksum.contents);
#endif                          /* MIT_NEW_CRYPTO */

    if (ivector.data)
        free(ivector.data);

    if (subkey)
        krb5_free_keyblock(kcontext, subkey);

    if (encrypted_data)
        free(encrypted_data);

    if (cc)
        krb5_cc_close(kcontext, cc);

    if (auth_context && !ksm_state)
        krb5_auth_con_free(kcontext, auth_context);

    return retval;
}

/****************************************************************************
 *
 * ksm_process_in_msg
 *
 * Parameters:
 *	(See list below...)
 *
 * Returns:
 *	KSM_ERR_NO_ERROR                        On success.
 *	SNMPERR_KRB5
 *	KSM_ERR_GENERIC_ERROR
 *	KSM_ERR_UNSUPPORTED_SECURITY_LEVEL
 *
 *
 * Processes an incoming message.
 *
 ****************************************************************************/

int
ksm_process_in_msg(struct snmp_secmod_incoming_params *parms)
{
    long            temp;
    krb5_cksumtype  cksumtype;
    krb5_auth_context auth_context = NULL;
    krb5_error_code retcode;
    krb5_checksum   checksum;
    krb5_data       ap_req, ivector;
    krb5_flags      flags;
    krb5_keyblock  *subkey = NULL;
#ifdef MIT_NEW_CRYPTO
    krb5_data       input, output;
    krb5_boolean    valid;
    krb5_enc_data   in_crypt;
#else                           /* MIT_NEW_CRYPTO */
    krb5_encrypt_block eblock;
#endif                          /* MIT_NEW_CRYPTO */
    krb5_ticket    *ticket = NULL;
    int             retval = SNMPERR_SUCCESS, response = 0;
    size_t          length =
        parms->wholeMsgLen - (u_int) (parms->secParams - parms->wholeMsg);
    u_char         *current = parms->secParams, type;
    size_t          cksumlength, blocksize;
    long            hint;
    char           *cname;
    struct ksm_secStateRef *ksm_state;
    struct ksm_cache_entry *entry;

    DEBUGMSGTL(("ksm", "Processing has begun\n"));

    checksum.contents = NULL;
    ap_req.data = NULL;
    ivector.length = 0;
    ivector.data = NULL;

    /*
     * First, parse the security parameters (because we need the subkey inside
     * of the ticket to do anything
     */

    if ((current = asn_parse_sequence(current, &length, &type,
                                      (ASN_UNIVERSAL | ASN_PRIMITIVE |
                                       ASN_OCTET_STR),
                                      "ksm first octet")) == NULL) {
        DEBUGMSGTL(("ksm", "Initial security paramter parsing failed\n"));

        retval = SNMPERR_ASN_PARSE_ERR;
        goto error;
    }

    if ((current = asn_parse_sequence(current, &length, &type,
                                      (ASN_SEQUENCE | ASN_CONSTRUCTOR),
                                      "ksm sequence")) == NULL) {
        DEBUGMSGTL(("ksm",
                    "Security parameter sequence parsing failed\n"));

        retval = SNMPERR_ASN_PARSE_ERR;
        goto error;
    }

    if ((current = asn_parse_int(current, &length, &type, &temp,
                                 sizeof(temp))) == NULL) {
        DEBUGMSGTL(("ksm", "Security parameter checksum type parsing"
                    "failed\n"));

        retval = SNMPERR_ASN_PARSE_ERR;
        goto error;
    }

    cksumtype = temp;

#ifdef MIT_NEW_CRYPTO
    if (!krb5_c_valid_cksumtype(cksumtype)) {
        DEBUGMSGTL(("ksm", "Invalid checksum type (%d)\n", cksumtype));

        retval = SNMPERR_KRB5;
        snmp_set_detail("Invalid checksum type");
        goto error;
    }

    if (!krb5_c_is_keyed_cksum(cksumtype)) {
        DEBUGMSGTL(("ksm", "Checksum type %d is not a keyed checksum\n",
                    cksumtype));
        snmp_set_detail("Checksum is not a keyed checksum");
        retval = SNMPERR_KRB5;
        goto error;
    }

    if (!krb5_c_is_coll_proof_cksum(cksumtype)) {
        DEBUGMSGTL(("ksm", "Checksum type %d is not a collision-proof "
                    "checksum\n", cksumtype));
        snmp_set_detail("Checksum is not a collision-proof checksum");
        retval = SNMPERR_KRB5;
        goto error;
    }
#else /* ! MIT_NEW_CRYPTO */
    if (!valid_cksumtype(cksumtype)) {
        DEBUGMSGTL(("ksm", "Invalid checksum type (%d)\n", cksumtype));

        retval = SNMPERR_KRB5;
        snmp_set_detail("Invalid checksum type");
        goto error;
    }

    if (!is_keyed_cksum(cksumtype)) {
        DEBUGMSGTL(("ksm", "Checksum type %d is not a keyed checksum\n",
                    cksumtype));
        snmp_set_detail("Checksum is not a keyed checksum");
        retval = SNMPERR_KRB5;
        goto error;
    }

    if (!is_coll_proof_cksum(cksumtype)) {
        DEBUGMSGTL(("ksm", "Checksum type %d is not a collision-proof "
                    "checksum\n", cksumtype));
        snmp_set_detail("Checksum is not a collision-proof checksum");
        retval = SNMPERR_KRB5;
        goto error;
    }
#endif /* MIT_NEW_CRYPTO */

    checksum.checksum_type = cksumtype;

    cksumlength = length;

    if ((current = asn_parse_sequence(current, &cksumlength, &type,
                                      (ASN_UNIVERSAL | ASN_PRIMITIVE |
                                       ASN_OCTET_STR), "ksm checksum")) ==
        NULL) {
        DEBUGMSGTL(("ksm",
                    "Security parameter checksum parsing failed\n"));

        retval = SNMPERR_ASN_PARSE_ERR;
        goto error;
    }

    checksum.contents = malloc(cksumlength);
    if (!checksum.contents) {
        DEBUGMSGTL(("ksm", "Unable to malloc %d bytes for checksum.\n",
                    cksumlength));
        retval = SNMPERR_MALLOC;
        goto error;
    }

    memcpy(checksum.contents, current, cksumlength);

    checksum.length = cksumlength;
    checksum.checksum_type = cksumtype;

    /*
     * Zero out the checksum so the validation works correctly
     */

    memset(current, 0, cksumlength);

    current += cksumlength;
    length = parms->wholeMsgLen - (u_int) (current - parms->wholeMsg);

    if ((current = asn_parse_sequence(current, &length, &type,
                                      (ASN_UNIVERSAL | ASN_PRIMITIVE |
                                       ASN_OCTET_STR), "ksm ap_req")) ==
        NULL) {
        DEBUGMSGTL(("ksm", "KSM security parameter AP_REQ/REP parsing "
                    "failed\n"));

        retval = SNMPERR_ASN_PARSE_ERR;
        goto error;
    }

    ap_req.length = length;
    ap_req.data = malloc(length);
    if (!ap_req.data) {
        DEBUGMSGTL(("ksm",
                    "KSM unable to malloc %d bytes for AP_REQ/REP.\n",
                    length));
        retval = SNMPERR_MALLOC;
        goto error;
    }

    memcpy(ap_req.data, current, length);

    current += length;
    length = parms->wholeMsgLen - (u_int) (current - parms->wholeMsg);

    if ((current = asn_parse_int(current, &length, &type, &hint,
                                 sizeof(hint))) == NULL) {
        DEBUGMSGTL(("ksm",
                    "KSM security parameter hint parsing failed\n"));

        retval = SNMPERR_ASN_PARSE_ERR;
        goto error;
    }

    /*
     * Okay!  We've got it all!  Now try decoding the damn ticket.
     *
     * But of course there's a WRINKLE!  We need to figure out if we're
     * processing a AP_REQ or an AP_REP.  How do we do that?  We're going
     * to cheat, and look at the first couple of bytes (which is what
     * the Kerberos library routines do anyway).
     *
     * If there are ever new Kerberos message formats, we'll need to fix
     * this here.
     *
     * If it's a _response_, then we need to get the auth_context
     * from our cache.
     */

    if (ap_req.length
        && (ap_req.data[0] == 0x6e || ap_req.data[0] == 0x4e)) {

        /*
         * We need to initalize the authorization context, and set the
         * replay cache in it (and initialize the replay cache if we
         * haven't already
         */

        retcode = krb5_auth_con_init(kcontext, &auth_context);

        if (retcode) {
            DEBUGMSGTL(("ksm", "krb5_auth_con_init failed: %s\n",
                        error_message(retcode)));
            retval = SNMPERR_KRB5;
            snmp_set_detail(error_message(retcode));
            goto error;
        }

        if (!rcache) {
            krb5_data       server;
            server.data = "host";
            server.length = strlen(server.data);

            retcode = krb5_get_server_rcache(kcontext, &server, &rcache);

            if (retcode) {
                DEBUGMSGTL(("ksm", "krb5_get_server_rcache failed: %s\n",
                            error_message(retcode)));
                retval = SNMPERR_KRB5;
                snmp_set_detail(error_message(retcode));
                goto error;
            }
        }

        retcode = krb5_auth_con_setrcache(kcontext, auth_context, rcache);

        if (retcode) {
            DEBUGMSGTL(("ksm", "krb5_auth_con_setrcache failed: %s\n",
                        error_message(retcode)));
            retval = SNMPERR_KRB5;
            snmp_set_detail(error_message(retcode));
            goto error;
        }

        retcode = krb5_rd_req(kcontext, &auth_context, &ap_req, NULL,
                              NULL, &flags, &ticket);

        krb5_auth_con_setrcache(kcontext, auth_context, NULL);

        if (retcode) {
            DEBUGMSGTL(("ksm", "krb5_rd_req() failed: %s\n",
                        error_message(retcode)));
            retval = SNMPERR_KRB5;
            snmp_set_detail(error_message(retcode));
            goto error;
        }

        retcode =
            krb5_unparse_name(kcontext, ticket->enc_part2->client, &cname);

        if (retcode == 0) {
            DEBUGMSGTL(("ksm", "KSM authenticated principal name: %s\n",
                        cname));
            free(cname);
        }

        /*
         * Check to make sure AP_OPTS_MUTUAL_REQUIRED was set
         */

        if (!(flags & AP_OPTS_MUTUAL_REQUIRED)) {
            DEBUGMSGTL(("ksm",
                        "KSM MUTUAL_REQUIRED not set in request!\n"));
            retval = SNMPERR_KRB5;
            snmp_set_detail("MUTUAL_REQUIRED not set in message");
            goto error;
        }

        retcode =
            krb5_auth_con_getremotesubkey(kcontext, auth_context, &subkey);

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM remote subkey retrieval failed: %s\n",
                        error_message(retcode)));
            retval = SNMPERR_KRB5;
            snmp_set_detail(error_message(retcode));
            goto error;
        }

    } else if (ap_req.length && (ap_req.data[0] == 0x6f ||
                                 ap_req.data[0] == 0x4f)) {
        /*
         * Looks like a response; let's see if we've got that auth_context
         * in our cache.
         */

        krb5_ap_rep_enc_part *repl = NULL;

        response = 1;

        entry = ksm_get_cache(parms->pdu->msgid);

        if (!entry) {
            DEBUGMSGTL(("ksm",
                        "KSM: Unable to find auth_context for PDU with "
                        "message ID of %ld\n", parms->pdu->msgid));
            retval = SNMPERR_KRB5;
            goto error;
        }

        auth_context = entry->auth_context;

        /*
         * In that case, let's call the rd_rep function
         */

        retcode = krb5_rd_rep(kcontext, auth_context, &ap_req, &repl);

        if (repl)
            krb5_free_ap_rep_enc_part(kcontext, repl);

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM: krb5_rd_rep() failed: %s\n",
                        error_message(retcode)));
            retval = SNMPERR_KRB5;
            goto error;
        }

        DEBUGMSGTL(("ksm", "KSM: krb5_rd_rep() decoded successfully.\n"));

        retcode =
            krb5_auth_con_getlocalsubkey(kcontext, auth_context, &subkey);

        if (retcode) {
            DEBUGMSGTL(("ksm", "Unable to retrieve local subkey: %s\n",
                        error_message(retcode)));
            retval = SNMPERR_KRB5;
            snmp_set_detail("Unable to retrieve local subkey");
            goto error;
        }

    } else {
        DEBUGMSGTL(("ksm", "Unknown Kerberos message type (%02x)\n",
                    ap_req.data[0]));
        retval = SNMPERR_KRB5;
        snmp_set_detail("Unknown Kerberos message type");
        goto error;
    }

#ifdef MIT_NEW_CRYPTO
    input.data = (char *) parms->wholeMsg;
    input.length = parms->wholeMsgLen;

    retcode =
        krb5_c_verify_checksum(kcontext, subkey, KSM_KEY_USAGE_CHECKSUM,
                               &input, &checksum, &valid);
#else                           /* MIT_NEW_CRYPTO */
    retcode = krb5_verify_checksum(kcontext, cksumtype, &checksum,
                                   parms->wholeMsg, parms->wholeMsgLen,
                                   (krb5_pointer) subkey->contents,
                                   subkey->length);
#endif                          /* MIT_NEW_CRYPTO */

    if (retcode) {
        DEBUGMSGTL(("ksm", "KSM checksum verification failed: %s\n",
                    error_message(retcode)));
        retval = SNMPERR_KRB5;
        snmp_set_detail(error_message(retcode));
        goto error;
    }

    /*
     * Don't ask me why they didn't simply return an error, but we have
     * to check to see if "valid" is false.
     */

#ifdef MIT_NEW_CRYPTO
    if (!valid) {
        DEBUGMSGTL(("ksm", "Computed checksum did not match supplied "
                    "checksum!\n"));
        retval = SNMPERR_KRB5;
        snmp_set_detail
            ("Computed checksum did not match supplied checksum");
        goto error;
    }
#endif                          /* MIT_NEW_CRYPTO */

    /*
     * Handle an encrypted PDU.  Note that it's an OCTET_STRING of the
     * output of whatever Kerberos cryptosystem you're using (defined by
     * the encryption type).  Note that this is NOT the EncryptedData
     * sequence - it's what goes in the "cipher" field of EncryptedData.
     */

    if (parms->secLevel == SNMP_SEC_LEVEL_AUTHPRIV) {

        if ((current = asn_parse_sequence(current, &length, &type,
                                          (ASN_UNIVERSAL | ASN_PRIMITIVE |
                                           ASN_OCTET_STR), "ksm pdu")) ==
            NULL) {
            DEBUGMSGTL(("ksm", "KSM sPDU octet decoding failed\n"));
            retval = SNMPERR_ASN_PARSE_ERR;
            goto error;
        }

        /*
         * The PDU is now pointed at by "current", and the length is in
         * "length".
         */

        DEBUGMSGTL(("ksm", "KSM starting sPDU decode\n"));

        /*
         * We need to set up a blank initialization vector for the decryption.
         * Use a block of all zero's (which is dependent on the block size
         * of the encryption method).
         */

#ifdef MIT_NEW_CRYPTO

        retcode = krb5_c_block_size(kcontext, subkey->enctype, &blocksize);

        if (retcode) {
            DEBUGMSGTL(("ksm",
                        "Unable to determine crypto block size: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }
#else                           /* MIT_NEW_CRYPTO */

        blocksize =
            krb5_enctype_array[subkey->enctype]->system->block_length;

#endif                          /* MIT_NEW_CRYPTO */

        ivector.data = malloc(blocksize);

        if (!ivector.data) {
            DEBUGMSGTL(("ksm", "Unable to allocate %d bytes for ivector\n",
                        blocksize));
            retval = SNMPERR_MALLOC;
            goto error;
        }

        ivector.length = blocksize;
        memset(ivector.data, 0, blocksize);

#ifndef MIT_NEW_CRYPTO

        krb5_use_enctype(kcontext, &eblock, subkey->enctype);

        retcode = krb5_process_key(kcontext, &eblock, subkey);

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM key post-processing failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }
#endif                          /* !MIT_NEW_CRYPTO */

        if (length > *parms->scopedPduLen) {
            DEBUGMSGTL(("ksm", "KSM not enough room - have %d bytes to "
                        "decrypt but only %d bytes available\n", length,
                        *parms->scopedPduLen));
            retval = SNMPERR_TOO_LONG;
#ifndef MIT_NEW_CRYPTO
            krb5_finish_key(kcontext, &eblock);
#endif                          /* ! MIT_NEW_CRYPTO */
            goto error;
        }
#ifdef MIT_NEW_CRYPTO
        in_crypt.ciphertext.data = (char *) current;
        in_crypt.ciphertext.length = length;
        in_crypt.enctype = subkey->enctype;
        output.data = (char *) *parms->scopedPdu;
        output.length = *parms->scopedPduLen;

        retcode =
            krb5_c_decrypt(kcontext, subkey, KSM_KEY_USAGE_ENCRYPTION,
                           &ivector, &in_crypt, &output);
#else                           /* MIT_NEW_CRYPTO */

        retcode = krb5_decrypt(kcontext, (krb5_pointer) current,
                               *parms->scopedPdu, length, &eblock,
                               ivector.data);

        krb5_finish_key(kcontext, &eblock);

#endif                          /* MIT_NEW_CRYPTO */

        if (retcode) {
            DEBUGMSGTL(("ksm", "Decryption failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }

        *parms->scopedPduLen = length;

    } else {
        /*
         * Clear PDU
         */

        *parms->scopedPdu = current;
        *parms->scopedPduLen =
            parms->wholeMsgLen - (current - parms->wholeMsg);
    }

    /*
     * A HUGE GROSS HACK
     */

    *parms->maxSizeResponse = parms->maxMsgSize - 200;

    DEBUGMSGTL(("ksm", "KSM processing complete\n"));

    /*
     * Set the secName to the right value (a hack for now).  But that's
     * only used for when we're processing a request, not a response.
     */

    if (!response) {

        retcode = krb5_unparse_name(kcontext, ticket->enc_part2->client,
                                    &cname);

        if (retcode) {
            DEBUGMSGTL(("ksm", "KSM krb5_unparse_name failed: %s\n",
                        error_message(retcode)));
            snmp_set_detail(error_message(retcode));
            retval = SNMPERR_KRB5;
            goto error;
        }

        if (strlen(cname) > *parms->secNameLen + 1) {
            DEBUGMSGTL(("ksm",
                        "KSM: Principal length (%d) is too long (%d)\n",
                        strlen(cname), parms->secNameLen));
            retval = SNMPERR_TOO_LONG;
            free(cname);
            goto error;
        }

        strcpy(parms->secName, cname);
        *parms->secNameLen = strlen(cname);

        free(cname);

        /*
         * Also, if we're not a response, keep around our auth_context so we
         * can encode the reply message correctly
         */

        ksm_state = SNMP_MALLOC_STRUCT(ksm_secStateRef);

        if (!ksm_state) {
            DEBUGMSGTL(("ksm", "KSM unable to malloc memory for "
                        "ksm_secStateRef\n"));
            retval = SNMPERR_MALLOC;
            goto error;
        }

        ksm_state->auth_context = auth_context;
        auth_context = NULL;
        ksm_state->cksumtype = cksumtype;

        *parms->secStateRef = ksm_state;
    } else {

        /*
         * We _still_ have to set the secName in process_in_msg().  Do
         * that now with what we were passed in before (we cached it,
         * remember?)
         */

        memcpy(parms->secName, entry->secName, entry->secNameLen);
        *parms->secNameLen = entry->secNameLen;
    }

    /*
     * Just in case
     */

    parms->secEngineID = (u_char *) "";
    *parms->secEngineIDLen = 0;

    auth_context = NULL;        /* So we don't try to free it on success */

  error:
    if (retval == SNMPERR_ASN_PARSE_ERR &&
        snmp_increment_statistic(STAT_SNMPINASNPARSEERRS) == 0)
        DEBUGMSGTL(("ksm", "Failed to increment statistics.\n"));

    if (subkey)
        krb5_free_keyblock(kcontext, subkey);

    if (checksum.contents)
        free(checksum.contents);

    if (ivector.data)
        free(ivector.data);

    if (ticket)
        krb5_free_ticket(kcontext, ticket);

    if (!response && auth_context)
        krb5_auth_con_free(kcontext, auth_context);

    if (ap_req.data)
        free(ap_req.data);

    return retval;
}
