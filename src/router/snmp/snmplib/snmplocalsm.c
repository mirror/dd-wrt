/*
 * snmplocalsm.c
 *
 * This code implements a security model that assumes the local user
 * that executed the agent is the user who's attributes called
 */

#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/library/snmplocalsm.h>

#include <unistd.h>

static int      localsm_session_init(netsnmp_session *);
static void     localsm_free_state_ref(void *);
static int      localsm_free_pdu(netsnmp_pdu *);
static int      localsm_clone_pdu(netsnmp_pdu *, netsnmp_pdu *);

u_int next_sess_id = 1;

/** Initialize the LOCALSM security module */
void
init_localsm(void)
{
    struct snmp_secmod_def *def;
    int ret;

    def = SNMP_MALLOC_STRUCT(snmp_secmod_def);

    if (!def) {
        snmp_log(LOG_ERR,
                 "Unable to malloc snmp_secmod struct, not registering LOCALSM\n");
        return;
    }

    def->encode_reverse = localsm_rgenerate_out_msg;
    def->decode = localsm_process_in_msg;
    def->session_open = localsm_session_init;
    def->pdu_free_state_ref = localsm_free_state_ref;
    def->pdu_free = localsm_free_pdu;
    def->pdu_clone = localsm_clone_pdu;

    DEBUGMSGTL(("localsm","registering ourselves\n"));
    ret = register_sec_mod(NETSNMP_LOCALSM_SECURITY_MODEL, "localsm", def);
    DEBUGMSGTL(("localsm"," returned %d\n", ret));
}

/*
 * Initialize specific session information (right now, just set up things to
 * not do an engineID probe)
 */

static int
localsm_session_init(netsnmp_session * sess)
{
    DEBUGMSGTL(("localsm",
                "LOCALSM: Reached our session initialization callback\n"));

    sess->flags |= SNMP_FLAGS_DONT_PROBE;

    /* XXX: likely needed for something: */
    /*
    localsmsession = sess->securityInfo =
    if (!localsmsession)
        return SNMPERR_GENERR;
    */

    return SNMPERR_SUCCESS;
}

/** Free our state information (this is only done on the agent side) */
static void
localsm_free_state_ref(void *ptr)
{
    free(ptr);
}

/** This is called when the PDU is freed. */
static int
localsm_free_pdu(netsnmp_pdu *pdu)
{
    return SNMPERR_SUCCESS;
}

/** This is called when a PDU is cloned (to increase reference counts) */
static int
localsm_clone_pdu(netsnmp_pdu *pdu, netsnmp_pdu *pdu2)
{
    return SNMPERR_SUCCESS;
}

/* asn.1 easing definitions */
#define LOCALSMBUILD_OR_ERR(fun, args, msg, desc)       \
    DEBUGDUMPHEADER("send", desc); \
    rc = fun args;            \
    DEBUGINDENTLESS();        \
    if (rc == 0) { \
        DEBUGMSGTL(("localsm",msg)); \
        retval = SNMPERR_TOO_LONG; \
        goto outerr; \
    }

#define BUILD_START_SEQ tmpoffset = *offset;

#define BUILD_END_SEQ(x) LOCALSMBUILD_OR_ERR(asn_realloc_rbuild_sequence, \
                         (wholeMsg, wholeMsgLen, offset, 1, \
                          (u_char) (ASN_SEQUENCE | ASN_CONSTRUCTOR), \
                          *offset - tmpoffset), \
                                          x, "sequence");

/****************************************************************************
 *
 * localsm_generate_out_msg
 *
 * Parameters:
 *	(See list below...)
 *
 * Returns:
 *	SNMPERR_SUCCESS                        On success.
 *	... and others
 *
 *
 * Generate an outgoing message.
 *
 ****************************************************************************/

int
localsm_rgenerate_out_msg(struct snmp_secmod_outgoing_params *parms)
{
    u_char         **wholeMsg = parms->wholeMsg;
    size_t	   *offset = parms->wholeMsgOffset;
    int rc;
    
    size_t         *wholeMsgLen = parms->wholeMsgLen;


    DEBUGMSGTL(("localsm", "Starting LOCALSM processing\n"));


    /*
     * We define here what the security message section will look like:
     * 04 00 -- null string
     * XXX: need to actually negotiate a context engine ID?
     * XXX: leave room for future expansion just in case?
     */
    DEBUGDUMPHEADER("send", "localsm security parameters");
    rc = asn_realloc_rbuild_header(wholeMsg, wholeMsgLen, offset, 1,
                                     (u_char) (ASN_UNIVERSAL | ASN_PRIMITIVE
                                             | ASN_OCTET_STR), 0);
    DEBUGINDENTLESS();
    if (rc == 0) {
        DEBUGMSGTL(("localsm", "building msgSecurityParameters failed.\n"));
        return SNMPERR_TOO_LONG;
    }
    
    /*
     * Copy in the msgGlobalData and msgVersion.  
     */
    while ((*wholeMsgLen - *offset) < parms->globalDataLen) {
        if (!asn_realloc(wholeMsg, wholeMsgLen)) {
            DEBUGMSGTL(("localsm", "building global data failed.\n"));
            return SNMPERR_TOO_LONG;
        }
    }

    *offset += parms->globalDataLen;
    memcpy(*wholeMsg + *wholeMsgLen - *offset,
           parms->globalData, parms->globalDataLen);

    /*
     * Total packet sequence.  
     */
    rc = asn_realloc_rbuild_sequence(wholeMsg, wholeMsgLen, offset, 1,
                                     (u_char) (ASN_SEQUENCE |
                                               ASN_CONSTRUCTOR), *offset);
    if (rc == 0) {
        DEBUGMSGTL(("localsm", "building master packet sequence failed.\n"));
        return SNMPERR_TOO_LONG;
    }

    DEBUGMSGTL(("localsm", "LOCALSM processing completed.\n"));
    return SNMPERR_SUCCESS;
}

/****************************************************************************
 *
 * localsm_process_in_msg
 *
 * Parameters:
 *	(See list below...)
 *
 * Returns:
 *	LOCALSM_ERR_NO_ERROR                        On success.
 *	LOCALSM_ERR_GENERIC_ERROR
 *	LOCALSM_ERR_UNSUPPORTED_SECURITY_LEVEL
 *
 *
 * Processes an incoming message.
 *
 ****************************************************************************/

int
localsm_process_in_msg(struct snmp_secmod_incoming_params *parms)
{
    u_char type_value;
    size_t octet_string_length;
    u_char *data_ptr;

    /* we don't have one, so set it to 0 */
    parms->secEngineID = strdup("");
    *parms->secEngineIDLen = 0;

    /* if this did not come through a tunneled connection, this
       security model is in appropriate (and would be a HUGE security
       hole to assume otherwise) */
    DEBUGMSGTL(("localsm","checking how we got here\n"));
    if (!(parms->pdu->flags & UCD_MSG_FLAG_TUNNELED)) {
        DEBUGMSGTL(("localsm","  not tunneled\n"));
        return SNMPERR_USM_AUTHENTICATIONFAILURE;
    } else {
        DEBUGMSGTL(("localsm","  tunneled\n"));
    }


    /*
     * Eat the first octet header.
     */
    if ((data_ptr = asn_parse_sequence(parms->secParams, &octet_string_length,
                                        &type_value,
                                        (ASN_UNIVERSAL | ASN_PRIMITIVE |
                                         ASN_OCTET_STR),
                                        "usm first octet")) == NULL) {
        /*
         * RETURN parse error 
         */
        return -1;
    }
    strncpy(parms->secName, strdup(getenv("USER")), *parms->secNameLen);
    *parms->secNameLen = strlen(parms->secName);
    DEBUGMSGTL(("localsm", "user: %s/%d\n", parms->secName, parms->secNameLen));
    
    *parms->scopedPdu = data_ptr;
    *parms->scopedPduLen = parms->wholeMsgLen - (data_ptr - parms->wholeMsg);
    *parms->maxSizeResponse = parms->maxMsgSize; /* XXX */
    parms->secEngineID = strdup("");
    *parms->secEngineIDLen = 0;
    parms->secLevel = SNMP_SEC_LEVEL_NOAUTH;
/*
 * maybe set this based on the transport in the future:
 *
*/

    if (octet_string_length != 0)
        return -1;

    return SNMPERR_SUCCESS;
}
