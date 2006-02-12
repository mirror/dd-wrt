/*
 * snmpv3.c
 */

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
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
#include <ctype.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_STDLIB_H
#       include <stdlib.h>
#endif

/*
 * Stuff needed for getHwAddress(...) 
 */
#ifdef HAVE_SYS_IOCTL_H
#	include <sys/ioctl.h>
#endif
#ifdef HAVE_NET_IF_H
#	include <net/if.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/snmpv3.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/lcd_time.h>
#include <net-snmp/library/scapi.h>
#include <net-snmp/library/keytools.h>
#include <net-snmp/library/lcd_time.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/snmpusm.h>
#include <net-snmp/library/transform_oids.h>

static u_long   engineBoots = 1;
static unsigned int engineIDType = ENGINEID_TYPE_UCD_RND;
static unsigned char *engineID = NULL;
static size_t   engineIDLength = 0;
static unsigned char *engineIDNic = NULL;
static unsigned int engineIDIsSet = 0;  /* flag if ID set by config */
static unsigned char *oldEngineID = NULL;
static size_t   oldEngineIDLength = 0;
static struct timeval snmpv3starttime;

/*
 * Set up default snmpv3 parameter value storage.
 */
static const oid *defaultAuthType = NULL;
static size_t   defaultAuthTypeLen = 0;
static const oid *defaultPrivType = NULL;
static size_t   defaultPrivTypeLen = 0;

#if defined(IFHWADDRLEN) && defined(SIOCGIFHWADDR)
static int      getHwAddress(const char *networkDevice, char *addressOut);
#endif

void
snmpv3_authtype_conf(const char *word, char *cptr)
{
    if (strcasecmp(cptr, "MD5") == 0)
        defaultAuthType = usmHMACMD5AuthProtocol;
    else if (strcasecmp(cptr, "SHA") == 0)
        defaultAuthType = usmHMACSHA1AuthProtocol;
    else
        config_perror("Unknown authentication type");
    defaultAuthTypeLen = USM_LENGTH_OID_TRANSFORM;
    DEBUGMSGTL(("snmpv3", "set default authentication type: %s\n", cptr));
}

const oid      *
get_default_authtype(size_t * len)
{
    if (defaultAuthType == NULL) {
        defaultAuthType = SNMP_DEFAULT_AUTH_PROTO;
        defaultAuthTypeLen = SNMP_DEFAULT_AUTH_PROTOLEN;
    }
    if (len)
        *len = defaultAuthTypeLen;
    return defaultAuthType;
}

void
snmpv3_privtype_conf(const char *word, char *cptr)
{
    if (strcasecmp(cptr, "DES") == 0)
        defaultPrivType = usmDESPrivProtocol;
#if HAVE_AES
    /* XXX AES: assumes oid length == des oid length */
    else if (strcasecmp(cptr, "AES128") == 0)
        defaultPrivType = usmAES128PrivProtocol;
    else if (strcasecmp(cptr, "AES192") == 0)
        defaultPrivType = usmAES192PrivProtocol;
    else if (strcasecmp(cptr, "AES256") == 0)
        defaultPrivType = usmAES256PrivProtocol;
#endif
    else
        config_perror("Unknown privacy type");
    defaultPrivTypeLen = SNMP_DEFAULT_PRIV_PROTOLEN;
    DEBUGMSGTL(("snmpv3", "set default privacy type: %s\n", cptr));
}

const oid      *
get_default_privtype(size_t * len)
{
    if (defaultPrivType == NULL) {
        defaultPrivType = usmDESPrivProtocol;
        defaultPrivTypeLen = USM_LENGTH_OID_TRANSFORM;
    }
    if (len)
        *len = defaultPrivTypeLen;
    return defaultPrivType;
}

/*******************************************************************-o-******
 * snmpv3_secLevel_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * Line syntax:
 *	defSecurityLevel "noAuthNoPriv" | "authNoPriv" | "authPriv"
 */
void
snmpv3_secLevel_conf(const char *word, char *cptr)
{
    char            buf[1024];

    if (strcasecmp(cptr, "noAuthNoPriv") == 0 || strcmp(cptr, "1") == 0 ||
	strcasecmp(cptr, "nanp") == 0) {
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, 
			   NETSNMP_DS_LIB_SECLEVEL, SNMP_SEC_LEVEL_NOAUTH);
    } else if (strcasecmp(cptr, "authNoPriv") == 0 || strcmp(cptr, "2") == 0 ||
	       strcasecmp(cptr, "anp") == 0) {
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, 
			   NETSNMP_DS_LIB_SECLEVEL, SNMP_SEC_LEVEL_AUTHNOPRIV);
    } else if (strcasecmp(cptr, "authPriv") == 0 || strcmp(cptr, "3") == 0 ||
	       strcasecmp(cptr, "ap") == 0) {
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, 
			   NETSNMP_DS_LIB_SECLEVEL, SNMP_SEC_LEVEL_AUTHPRIV);
    } else {
        snprintf(buf, sizeof(buf), "Unknown security level: %s", cptr);
        buf[ sizeof(buf)-1 ] = 0;
        config_perror(buf);
    }
    DEBUGMSGTL(("snmpv3", "default secLevel set to: %s = %d\n", cptr,
                netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
				   NETSNMP_DS_LIB_SECLEVEL)));
}


int
snmpv3_options(char *optarg, netsnmp_session * session, char **Apsz,
               char **Xpsz, int argc, char *const *argv)
{
    char           *cp = optarg;
    optarg++;
    /*
     * Support '... -3x=value ....' syntax
     */
    if (*optarg == '=') {
        optarg++;
    }
    /*
     * and '.... "-3x value" ....'  (*with* the quotes)
     */
    while (*optarg && isspace(*optarg)) {
        optarg++;
    }
    /*
     * Finally, handle ".... -3x value ...." syntax
     *   (*without* surrounding quotes)
     */
    if (!*optarg) {
        /*
         * We've run off the end of the argument
         *  so move on the the next.
         */
        optarg = argv[optind++];
        if (optind > argc) {
            fprintf(stderr,
                    "Missing argument after SNMPv3 '-3%c' option.\n", *cp);
            return (-1);
        }
    }

    switch (*cp) {

    case 'Z':
        session->engineBoots = strtoul(optarg, NULL, 10);
        if (session->engineBoots == 0 || !isdigit(optarg[0])) {
            fprintf(stderr, "Need engine boots value after -3Z flag.\n");
            return (-1);
        }
        cp = strchr(optarg, ',');
        if (cp && *(++cp) && isdigit(*cp))
            session->engineTime = strtoul(cp, NULL, 10);
        else {
            fprintf(stderr, "Need engine time value after -3Z flag.\n");
            return (-1);
        }
        break;

    case 'e':{
            size_t          ebuf_len = 32, eout_len = 0;
            u_char         *ebuf = (u_char *) malloc(ebuf_len);

            if (ebuf == NULL) {
                fprintf(stderr, "malloc failure processing -3e flag.\n");
                return (-1);
            }
            if (!snmp_hex_to_binary
                (&ebuf, &ebuf_len, &eout_len, 1, optarg)) {
                fprintf(stderr, "Bad engine ID value after -3e flag.\n");
                free(ebuf);
                return (-1);
            }
            session->securityEngineID = ebuf;
            session->securityEngineIDLen = eout_len;
            break;
        }

    case 'E':{
            size_t          ebuf_len = 32, eout_len = 0;
            u_char         *ebuf = (u_char *) malloc(ebuf_len);

            if (ebuf == NULL) {
                fprintf(stderr, "malloc failure processing -3E flag.\n");
                return (-1);
            }
            if (!snmp_hex_to_binary
                (&ebuf, &ebuf_len, &eout_len, 1, optarg)) {
                fprintf(stderr, "Bad engine ID value after -3E flag.\n");
                free(ebuf);
                return (-1);
            }
            session->contextEngineID = ebuf;
            session->contextEngineIDLen = eout_len;
            break;
        }

    case 'n':
        session->contextName = optarg;
        session->contextNameLen = strlen(optarg);
        break;

    case 'u':
        session->securityName = optarg;
        session->securityNameLen = strlen(optarg);
        break;

    case 'l':
        if (!strcasecmp(optarg, "noAuthNoPriv") || !strcmp(optarg, "1") ||
            !strcasecmp(optarg, "nanp")) {
            session->securityLevel = SNMP_SEC_LEVEL_NOAUTH;
        } else if (!strcasecmp(optarg, "authNoPriv")
                   || !strcmp(optarg, "2") || !strcasecmp(optarg, "anp")) {
            session->securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        } else if (!strcasecmp(optarg, "authPriv") || !strcmp(optarg, "3")
                   || !strcasecmp(optarg, "ap")) {
            session->securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
        } else {
            fprintf(stderr,
                    "Invalid security level specified after -3l flag: %s\n",
                    optarg);
            return (-1);
        }

        break;

    case 'a':
        if (!strcasecmp(optarg, "MD5")) {
            session->securityAuthProto = usmHMACMD5AuthProtocol;
            session->securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
        } else if (!strcasecmp(optarg, "SHA")) {
            session->securityAuthProto = usmHMACSHA1AuthProtocol;
            session->securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
        } else {
            fprintf(stderr,
                    "Invalid authentication protocol specified after -3a flag: %s\n",
                    optarg);
            return (-1);
        }
        break;

    case 'x':
        if (!strcasecmp(optarg, "DES")) {
            session->securityPrivProto = usmDESPrivProtocol;
            session->securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
#ifdef HAVE_AES
        } else if (!strcasecmp(optarg, "AES128")) {
            session->securityPrivProto = usmAES128PrivProtocol;
            session->securityPrivProtoLen = USM_PRIV_PROTO_AES128_LEN;
        } else if (!strcasecmp(optarg, "AES192")) {
            session->securityPrivProto = usmAES192PrivProtocol;
            session->securityPrivProtoLen = USM_PRIV_PROTO_AES192_LEN;
        } else if (!strcasecmp(optarg, "AES256")) {
            session->securityPrivProto = usmAES256PrivProtocol;
            session->securityPrivProtoLen = USM_PRIV_PROTO_AES256_LEN;
#endif
        } else {
            fprintf(stderr,
                    "Invalid privacy protocol specified after -3x flag: %s\n",
                    optarg);
            return (-1);
        }
        break;

    case 'A':
        *Apsz = optarg;
        break;

    case 'X':
        *Xpsz = optarg;
        break;

    default:
        fprintf(stderr, "Unknown SNMPv3 option passed to -3: %c.\n", *cp);
        return -1;
    }
    return 0;
}

/*******************************************************************-o-******
 * setup_engineID
 *
 * Parameters:
 *	**eidp
 *	 *text	Printable (?) text to be plugged into the snmpEngineID.
 *
 * Return:
 *	Length of allocated engineID string in bytes,  -OR-
 *	-1 on error.
 *
 *
 * Create an snmpEngineID using text and the local IP address.  If eidp
 * is defined, use it to return a pointer to the newly allocated data.
 * Otherwise, use the result to define engineID defined in this module.
 *
 * Line syntax:
 *	engineID <text> | NULL
 *
 * XXX	What if a node has multiple interfaces?
 * XXX	What if multiple engines all choose the same address?
 *      (answer:  You're screwed, because you might need a kul database
 *       which is dependant on the current engineID.  Enumeration and other
 *       tricks won't work). 
 */
int
setup_engineID(u_char ** eidp, const char *text)
{
    int             enterpriseid = htonl(ENTERPRISE_OID),
        ucdavisid = htonl(UCDAVIS_OID), localsetup = (eidp) ? 0 : 1;

    /*
     * Use local engineID if *eidp == NULL.  
     */
#ifdef HAVE_GETHOSTNAME
    u_char          buf[SNMP_MAXBUF_SMALL];
    struct hostent *hent;
#endif
    u_char         *bufp = NULL;
    size_t          len;
    int             localEngineIDType = engineIDType;
    int             tmpint;
    time_t          tmptime;

    engineIDIsSet = 1;

    /*
     * get the host name and save the information 
     */
#ifdef HAVE_GETHOSTNAME
    gethostname((char *) buf, sizeof(buf));
    hent = gethostbyname((char *) buf);
    /*
     * Determine if we are using IPV6 
     */
#ifdef AF_INET6
    /*
     * see if they selected IPV4 or IPV6 support 
     */
    if ((ENGINEID_TYPE_IPV6 == localEngineIDType) ||
        (ENGINEID_TYPE_IPV4 == localEngineIDType)) {
        if (hent && hent->h_addrtype == AF_INET6) {
            localEngineIDType = ENGINEID_TYPE_IPV6;
        } else {
            /*
             * Not IPV6 so we go with default 
             */
            localEngineIDType = ENGINEID_TYPE_IPV4;
        }
    }
#else
    /*
     * No IPV6 support.  Check if they selected IPV6 engineID type.  If so
     * * make it IPV4 for them 
     */
    if (ENGINEID_TYPE_IPV6 == localEngineIDType) {
        localEngineIDType = ENGINEID_TYPE_IPV4;
    }
#endif
#endif                          /* HAVE_GETHOSTNAME */

    /*
     * Determine if we have text and if so setup our localEngineIDType
     * * appropriately.  
     */
    if (NULL != text) {
        engineIDType = localEngineIDType = ENGINEID_TYPE_TEXT;
    }
    /*
     * Determine length of the engineID string. 
     */
    len = 5;                    /* always have 5 leading bytes */
    switch (localEngineIDType) {
    case ENGINEID_TYPE_TEXT:
        len += strlen(text);    /* 5 leading bytes+text. No NULL char */
        break;
#if defined(IFHWADDRLEN) && defined(SIOCGIFHWADDR)
    case ENGINEID_TYPE_MACADDR:        /* MAC address */
        len += 6;               /* + 6 bytes for MAC address */
        break;
#endif
    case ENGINEID_TYPE_IPV4:   /* IPv4 */
        len += 4;               /* + 4 byte IPV4 address */
        break;
    case ENGINEID_TYPE_IPV6:   /* IPv6 */
        len += 16;              /* + 16 byte IPV6 address */
        break;
    case ENGINEID_TYPE_UCD_RND:        /* UCD specific encoding */
        if (engineID)           /* already setup, keep current value */
            return engineIDLength;
        if (oldEngineID) {
            len = oldEngineIDLength;
        } else {
            len += sizeof(int) + sizeof(time_t);
        }
        break;
    default:
        snmp_log(LOG_ERR,
                 "Unknown EngineID type requested for setup (%d).  Using IPv4.\n",
                 localEngineIDType);
        localEngineIDType = ENGINEID_TYPE_IPV4; /* make into IPV4 */
        len += 4;               /* + 4 byte IPv4 address */
        break;
    }                           /* switch */


    /*
     * Allocate memory and store enterprise ID.
     */
    if ((bufp = (u_char *) malloc(len)) == NULL) {
        snmp_log_perror("setup_engineID malloc");
        return -1;
    }
    if (localEngineIDType == ENGINEID_TYPE_UCD_RND)
        /*
         * we must use the net-snmp enterprise id here, regardless 
         */
        memcpy(bufp, &ucdavisid, sizeof(ucdavisid));    /* XXX Must be 4 bytes! */
    else
        memcpy(bufp, &enterpriseid, sizeof(enterpriseid));      /* XXX Must be 4 bytes! */

    bufp[0] |= 0x80;


    /*
     * Store the given text  -OR-   the first found IP address.
     */
    switch (localEngineIDType) {
    case ENGINEID_TYPE_UCD_RND:
        if (oldEngineID) {
            /*
             * keep our previous notion of the engineID 
             */
            memcpy(bufp, oldEngineID, oldEngineIDLength);
        } else {
            /*
             * Here we've desigend our own ENGINEID that is not based on
             * an address which may change and may even become conflicting
             * in the future like most of the default v3 engineID types
             * suffer from.
             * 
             * Ours is built from 2 fairly random elements: a random number and
             * the current time in seconds.  This method suffers from boxes
             * that may not have a correct clock setting and random number
             * seed at startup, but few OSes should have that problem.
             */
            bufp[4] = ENGINEID_TYPE_UCD_RND;
            tmpint = random();
            memcpy(bufp + 5, &tmpint, sizeof(tmpint));
            tmptime = time(NULL);
            memcpy(bufp + 5 + sizeof(tmpint), &tmptime, sizeof(tmptime));
        }
        break;
    case ENGINEID_TYPE_TEXT:
        bufp[4] = ENGINEID_TYPE_TEXT;
        memcpy((char *) bufp + 5, text, strlen(text));
        break;
#ifdef HAVE_GETHOSTNAME
#ifdef AF_INET6
    case ENGINEID_TYPE_IPV6:
        bufp[4] = ENGINEID_TYPE_IPV6;
        memcpy(bufp + 5, hent->h_addr_list[0], hent->h_length);
        break;
#endif
#endif
#if defined(IFHWADDRLEN) && defined(SIOCGIFHWADDR)
    case ENGINEID_TYPE_MACADDR:
        {
            int             x;
            bufp[4] = ENGINEID_TYPE_MACADDR;
            /*
             * use default NIC if none provided 
             */
            if (NULL == engineIDNic) {
                x = getHwAddress(DEFAULT_NIC, &bufp[5]);
            } else {
                x = getHwAddress(engineIDNic, &bufp[5]);
            }
            if (0 != x)
                /*
                 * function failed fill MAC address with zeros 
                 */
            {
                memset(&bufp[5], 0, 6);
            }
        }
        break;
#endif
    case ENGINEID_TYPE_IPV4:
    default:
        bufp[4] = ENGINEID_TYPE_IPV4;
#ifdef HAVE_GETHOSTNAME
        if (hent && hent->h_addrtype == AF_INET) {
            memcpy(bufp + 5, hent->h_addr_list[0], hent->h_length);
        } else {                /* Unknown address type.  Default to 127.0.0.1. */

            bufp[5] = 127;
            bufp[6] = 0;
            bufp[7] = 0;
            bufp[8] = 1;
        }
#else                           /* HAVE_GETHOSTNAME */
        /*
         * Unknown address type.  Default to 127.0.0.1. 
         */
        bufp[5] = 127;
        bufp[6] = 0;
        bufp[7] = 0;
        bufp[8] = 1;
#endif                          /* HAVE_GETHOSTNAME */
        break;
    }

    /*
     * Pass the string back to the calling environment, or use it for
     * our local engineID.
     */
    if (localsetup) {
        SNMP_FREE(engineID);
        engineID = bufp;
        engineIDLength = len;

    } else {
        *eidp = bufp;
    }


    return len;

}                               /* end setup_engineID() */

void
usm_parse_create_usmUser(const char *token, char *line)
{
    char           *cp;
    char            buf[SNMP_MAXBUF_MEDIUM];
    struct usmUser *newuser;
    u_char          userKey[SNMP_MAXBUF_SMALL];
    size_t          userKeyLen = SNMP_MAXBUF_SMALL;
    size_t          ret;

    newuser = usm_create_user();

    /*
     * READ: Security Name 
     */
    cp = copy_nword(line, buf, sizeof(buf));

    /*
     * might be a -e ENGINEID argument 
     */
    if (strcmp(buf, "-e") == 0) {
        size_t          ebuf_len = 32, eout_len = 0;
        u_char         *ebuf = (u_char *) malloc(ebuf_len);

        if (ebuf == NULL) {
            config_perror("malloc failure processing -e flag");
            usm_free_user(newuser);
            return;
        }

        /*
         * Get the specified engineid from the line.  
         */
        cp = copy_nword(cp, buf, sizeof(buf));
        if (!snmp_hex_to_binary(&ebuf, &ebuf_len, &eout_len, 1, buf)) {
            config_perror("invalid EngineID argument to -e");
            usm_free_user(newuser);
            free(ebuf);
            return;
        }

        newuser->engineID = ebuf;
        newuser->engineIDLen = eout_len;
        cp = copy_nword(cp, buf, sizeof(buf));
    } else {
        newuser->engineID = snmpv3_generate_engineID(&ret);
        if (ret == 0) {
            usm_free_user(newuser);
            return;
        }
        newuser->engineIDLen = ret;
    }

    newuser->secName = strdup(buf);
    newuser->name = strdup(buf);

    if (!cp)
        goto add;               /* no authentication or privacy type */

    /*
     * READ: Authentication Type 
     */
    if (strncmp(cp, "MD5", 3) == 0) {
        memcpy(newuser->authProtocol, usmHMACMD5AuthProtocol,
               sizeof(usmHMACMD5AuthProtocol));
    } else if (strncmp(cp, "SHA", 3) == 0) {
        memcpy(newuser->authProtocol, usmHMACSHA1AuthProtocol,
               sizeof(usmHMACSHA1AuthProtocol));
    } else {
        config_perror("Unknown authentication protocol");
        usm_free_user(newuser);
        return;
    }

    cp = skip_token(cp);

    /*
     * READ: Authentication Pass Phrase 
     */
    if (!cp) {
        config_perror("no authentication pass phrase");
        usm_free_user(newuser);
        return;
    }
    cp = copy_nword(cp, buf, sizeof(buf));
    /*
     * And turn it into a localized key 
     */
    ret = generate_Ku(newuser->authProtocol, newuser->authProtocolLen,
                      (u_char *) buf, strlen(buf), userKey, &userKeyLen);
    if (ret != SNMPERR_SUCCESS) {
        config_perror("could not generate the authentication key from the "
                      "suppiled pass phrase.");
        usm_free_user(newuser);
        return;
    }
    newuser->authKeyLen =
        sc_get_properlength(newuser->authProtocol,
                            newuser->authProtocolLen);
    newuser->authKey = (u_char *) malloc(newuser->authKeyLen);
    ret = generate_kul(newuser->authProtocol, newuser->authProtocolLen,
                       newuser->engineID, newuser->engineIDLen,
                       userKey, userKeyLen,
                       newuser->authKey, &newuser->authKeyLen);
    if (ret != SNMPERR_SUCCESS) {
        config_perror("could not generate localized authentication key (Kul) "
                      "from the master key (Ku).");
        usm_free_user(newuser);
        return;
    }

    if (!cp)
        goto add;               /* no privacy type (which is legal) */

    /*
     * READ: Privacy Type 
     */
    if (strncmp(cp, "DES", 3) == 0) {
        memcpy(newuser->privProtocol, usmDESPrivProtocol,
               sizeof(usmDESPrivProtocol));
#ifdef HAVE_AES
    } else if (strncmp(cp, "AES128", 3) == 0) {
        memcpy(newuser->privProtocol, usmAES128PrivProtocol,
               sizeof(usmAES128PrivProtocol));
    } else if (strncmp(cp, "AES192", 3) == 0) {
        memcpy(newuser->privProtocol, usmAES192PrivProtocol,
               sizeof(usmAES192PrivProtocol));
    } else if (strncmp(cp, "AES256", 3) == 0) {
        memcpy(newuser->privProtocol, usmAES256PrivProtocol,
               sizeof(usmAES256PrivProtocol));
#endif
    } else {
        config_perror("Unknown privacy protocol");
        usm_free_user(newuser);
        return;
    }

    cp = skip_token(cp);
    /*
     * READ: Authentication Pass Phrase 
     */
    if (!cp) {
        /*
         * assume the same as the authentication key 
         */
        memdup(&newuser->privKey, newuser->authKey, newuser->authKeyLen);
        newuser->privKeyLen = newuser->authKeyLen;
    } else {
        cp = copy_nword(cp, buf, sizeof(buf));
        /*
         * And turn it into a localized key 
         */
        ret = generate_Ku(newuser->authProtocol, newuser->authProtocolLen,
                          (u_char *) buf, strlen(buf),
                          userKey, &userKeyLen);
        if (ret != SNMPERR_SUCCESS) {
            config_perror("could not generate privacy key from the supplied "
                          "pass phrase.");
            usm_free_user(newuser);
            return;
        }

        ret =
            sc_get_properlength(newuser->authProtocol,
                                newuser->authProtocolLen);
        if (ret < 0) {
            config_perror("could not get proper key length to use for the "
                          "privacy algorithm.");
            usm_free_user(newuser);
            return;
        }
        newuser->privKeyLen = ret;

        newuser->privKey = (u_char *) malloc(newuser->privKeyLen);
        ret = generate_kul(newuser->authProtocol, newuser->authProtocolLen,
                           newuser->engineID, newuser->engineIDLen,
                           userKey, userKeyLen,
                           newuser->privKey, &newuser->privKeyLen);
        if (ret != SNMPERR_SUCCESS) {
            config_perror("could not generate the localized privacy key (Kul) "
                          "from the master key (Ku).");
            usm_free_user(newuser);
            return;
        }
    }
  add:
    usm_add_user(newuser);
    DEBUGMSGTL(("usmUser", "created a new user %s at ", newuser->secName));
    DEBUGMSGHEX(("usmUser", newuser->engineID, newuser->engineIDLen));
    DEBUGMSG(("usmUser", "\n"));
}

/*******************************************************************-o-******
 * engineBoots_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * Line syntax:
 *	engineBoots <num_boots>
 */
void
engineBoots_conf(const char *word, char *cptr)
{
    engineBoots = atoi(cptr) + 1;
    DEBUGMSGTL(("snmpv3", "engineBoots: %d\n", engineBoots));
}

/*******************************************************************-o-******
 * engineIDType_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * Line syntax:
 *	engineIDType <1 or 3>
 *		1 is default for IPv4 engine ID type.  Will automatically
 *		    chose between IPv4 & IPv6 if either 1 or 2 is specified.
 *		2 is for IPv6.
 *		3 is hardware (MAC) address, currently supported under Linux
 */
void
engineIDType_conf(const char *word, char *cptr)
{
    engineIDType = atoi(cptr);
    /*
     * verify valid type selected 
     */
    switch (engineIDType) {
    case ENGINEID_TYPE_IPV4:   /* IPv4 */
    case ENGINEID_TYPE_IPV6:   /* IPv6 */
        /*
         * IPV? is always good 
         */
        break;
#if defined(IFHWADDRLEN) && defined(SIOCGIFHWADDR)
    case ENGINEID_TYPE_MACADDR:        /* MAC address */
        break;
#endif
    default:
        /*
         * unsupported one chosen 
         */
        config_perror("Unsupported enginedIDType, forcing IPv4");
        engineIDType = ENGINEID_TYPE_IPV4;
    }
    DEBUGMSGTL(("snmpv3", "engineIDType: %d\n", engineIDType));
}

/*******************************************************************-o-******
 * engineIDNic_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * Line syntax:
 *	engineIDNic <string>
 *		eth0 is default
 */
void
engineIDNic_conf(const char *word, char *cptr)
{
    /*
     * Make sure they haven't already specified the engineID via the
     * * configuration file 
     */
    if (0 == engineIDIsSet)
        /*
         * engineID has NOT been set via configuration file 
         */
    {
        /*
         * See if already set if so erase & release it 
         */
        if (NULL != engineIDNic) {
            free(engineIDNic);
        }
        engineIDNic = (u_char *) malloc(strlen(cptr) + 1);
        if (NULL != engineIDNic) {
            strcpy((char *) engineIDNic, cptr);
            DEBUGMSGTL(("snmpv3", "Initializing engineIDNic: %s\n",
                        engineIDNic));
        } else {
            DEBUGMSGTL(("snmpv3",
                        "Error allocating memory for engineIDNic!\n"));
        }
    } else {
        DEBUGMSGTL(("snmpv3",
                    "NOT setting engineIDNic, engineID already set\n"));
    }
}

/*******************************************************************-o-******
 * engineID_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * This function reads a string from the configuration file and uses that
 * string to initialize the engineID.  It's assumed to be human readable.
 */
void
engineID_conf(const char *word, char *cptr)
{
    setup_engineID(NULL, cptr);
    DEBUGMSGTL(("snmpv3", "initialized engineID with: %s\n", cptr));
}

void
version_conf(const char *word, char *cptr)
{
    if ((strcmp(cptr,  "1") == 0) ||
        (strcmp(cptr, "v1") == 0)) {
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_SNMPVERSION, 
			   NETSNMP_DS_SNMP_VERSION_1);       /* bogus value */
    } else if ((strcasecmp(cptr,  "2c") == 0) ||
               (strcasecmp(cptr, "v2c") == 0)) {
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_SNMPVERSION, 
			   NETSNMP_DS_SNMP_VERSION_2c);
    } else if ((strcasecmp(cptr,  "3" ) == 0) ||
               (strcasecmp(cptr, "v3" ) == 0)) {
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_SNMPVERSION, 
			   NETSNMP_DS_SNMP_VERSION_3);
    } else {
        config_perror("Unknown version specification");
        return;
    }
    DEBUGMSGTL(("snmpv3", "set default version to %d\n",
                netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
				   NETSNMP_DS_LIB_SNMPVERSION)));
}

/*
 * engineID_old_conf(const char *, char *):
 * 
 * Reads a octet string encoded engineID into the oldEngineID and
 * oldEngineIDLen pointers.
 */
void
oldengineID_conf(const char *word, char *cptr)
{
    read_config_read_octet_string(cptr, &oldEngineID, &oldEngineIDLength);
}


/*******************************************************************-o-******
 * init_snmpv3
 *
 * Parameters:
 *	*type	Label for the config file "type" used by calling entity.
 *      
 * Set time and engineID.
 * Set parsing functions for config file tokens.
 * Initialize SNMP Crypto API (SCAPI).
 */
void
init_snmpv3(const char *type)
{

    gettimeofday(&snmpv3starttime, NULL);

    if (!type)
        type = "__snmpapp__";

    /*
     * we need to be called back later 
     */
    snmp_register_callback(SNMP_CALLBACK_LIBRARY,
                           SNMP_CALLBACK_POST_READ_CONFIG,
                           init_snmpv3_post_config, NULL);
    snmp_register_callback(SNMP_CALLBACK_LIBRARY,
                           SNMP_CALLBACK_POST_PREMIB_READ_CONFIG,
                           init_snmpv3_post_premib_config, NULL);
    /*
     * we need to be called back later 
     */
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           snmpv3_store, (void *) strdup(type));

    /*
     * initialize submodules 
     */
    /*
     * NOTE: this must be after the callbacks are registered above,
     * since they need to be called before the USM callbacks. 
     */
    init_secmod();

    /*
     * register all our configuration handlers (ack, there's a lot) 
     */

    /*
     * handle engineID setup before everything else which may depend on it 
     */
    register_prenetsnmp_mib_handler(type, "engineID", engineID_conf, NULL,
                                    "string");
    register_prenetsnmp_mib_handler(type, "oldEngineID", oldengineID_conf,
                                    NULL, NULL);
    register_prenetsnmp_mib_handler(type, "engineIDType",
                                    engineIDType_conf, NULL, "num");
    register_prenetsnmp_mib_handler(type, "engineIDNic", engineIDNic_conf,
                                    NULL, "string");
    register_config_handler(type, "engineBoots", engineBoots_conf, NULL,
                            NULL);

    /*
     * default store config entries 
     */
    netsnmp_ds_register_config(ASN_OCTET_STR, "snmp", "defSecurityName",
			       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_SECNAME);
    netsnmp_ds_register_config(ASN_OCTET_STR, "snmp", "defContext", 
			       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CONTEXT);
    netsnmp_ds_register_config(ASN_OCTET_STR, "snmp", "defPassphrase",
			     NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PASSPHRASE);
    netsnmp_ds_register_config(ASN_OCTET_STR, "snmp", "defAuthPassphrase",
			 NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_AUTHPASSPHRASE);
    netsnmp_ds_register_config(ASN_OCTET_STR, "snmp", "defPrivPassphrase",
			 NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRIVPASSPHRASE);
    register_config_handler("snmp", "defVersion", version_conf, NULL,
                            "1|2c|3");

    register_config_handler("snmp", "defAuthType", snmpv3_authtype_conf,
                            NULL, "MD5|SHA");
    register_config_handler("snmp", "defPrivType", snmpv3_privtype_conf,
                            NULL,
#ifdef HAVE_AES
                            "DES (AES support not available)");
#else
                            "DES|AES128|AES192|AES256");
#endif
    register_config_handler("snmp", "defSecurityLevel",
                            snmpv3_secLevel_conf, NULL,
                            "noAuthNoPriv|authNoPriv|authPriv");
    register_config_handler(type, "userSetAuthPass", usm_set_password,
                            NULL, NULL);
    register_config_handler(type, "userSetPrivPass", usm_set_password,
                            NULL, NULL);
    register_config_handler(type, "userSetAuthKey", usm_set_password, NULL,
                            NULL);
    register_config_handler(type, "userSetPrivKey", usm_set_password, NULL,
                            NULL);
    register_config_handler(type, "userSetAuthLocalKey", usm_set_password,
                            NULL, NULL);
    register_config_handler(type, "userSetPrivLocalKey", usm_set_password,
                            NULL, NULL);
}

/*
 * initializations for SNMPv3 to be called after the configuration files
 * have been read.
 */

int
init_snmpv3_post_config(int majorid, int minorid, void *serverarg,
                        void *clientarg)
{

    size_t          engineIDLen;
    u_char         *c_engineID;

    c_engineID = snmpv3_generate_engineID(&engineIDLen);

    if (engineIDLen == 0) {
        /*
         * Somethine went wrong - help! 
         */
        return SNMPERR_GENERR;
    }

    /*
     * if our engineID has changed at all, the boots record must be set to 1 
     */
    if (engineIDLen != (int) oldEngineIDLength ||
        oldEngineID == NULL || c_engineID == NULL ||
        memcmp(oldEngineID, c_engineID, engineIDLen) != 0) {
        engineBoots = 1;
    }

    /*
     * set our local engineTime in the LCD timing cache 
     */
    set_enginetime(c_engineID, engineIDLen,
                   snmpv3_local_snmpEngineBoots(),
                   snmpv3_local_snmpEngineTime(), TRUE);

    free(c_engineID);
    return SNMPERR_SUCCESS;
}

int
init_snmpv3_post_premib_config(int majorid, int minorid, void *serverarg,
                               void *clientarg)
{
    if (!engineIDIsSet)
        setup_engineID(NULL, NULL);

    return SNMPERR_SUCCESS;
}

/*******************************************************************-o-******
 * store_snmpv3
 *
 * Parameters:
 *	*type
 */
int
snmpv3_store(int majorID, int minorID, void *serverarg, void *clientarg)
{
    char            line[SNMP_MAXBUF_SMALL];
    u_char          c_engineID[SNMP_MAXBUF_SMALL];
    int             engineIDLen;
    const char     *type = (const char *) clientarg;

    if (type == NULL)           /* should never happen, since the arg is ours */
        type = "unknown";

    sprintf(line, "engineBoots %ld", engineBoots);
    read_config_store(type, line);

    engineIDLen = snmpv3_get_engineID(c_engineID, SNMP_MAXBUF_SMALL);

    if (engineIDLen) {
        /*
         * store the engineID used for this run 
         */
        sprintf(line, "oldEngineID ");
        read_config_save_octet_string(line + strlen(line), c_engineID,
                                      engineIDLen);
        read_config_store(type, line);
    }
    return SNMPERR_SUCCESS;
}                               /* snmpv3_store() */

u_long
snmpv3_local_snmpEngineBoots(void)
{
    return engineBoots;
}


/*******************************************************************-o-******
 * snmpv3_get_engineID
 *
 * Parameters:
 *	*buf
 *	 buflen
 *      
 * Returns:
 *	Length of engineID	On Success
 *	SNMPERR_GENERR		Otherwise.
 *
 *
 * Store engineID in buf; return the length.
 *
 */
size_t
snmpv3_get_engineID(u_char * buf, size_t buflen)
{
    /*
     * Sanity check.
     */
    if (!buf || (buflen < engineIDLength)) {
        return 0;
    }

    memcpy(buf, engineID, engineIDLength);
    return engineIDLength;

}                               /* end snmpv3_get_engineID() */

/*******************************************************************-o-******
 * snmpv3_clone_engineID
 *
 * Parameters:
 *	**dest
 *       *dest_len
 *       src
 *	 srclen
 *      
 * Returns:
 *	Length of engineID	On Success
 *	0		        Otherwise.
 *
 *
 * Clones engineID, creates memory
 *
 */
int
snmpv3_clone_engineID(u_char ** dest, size_t * destlen, u_char * src,
                      size_t srclen)
{
    if (!dest || !destlen)
        return 0;

    if (*dest) {
        SNMP_FREE(*dest);
        *dest = NULL;
    }
    *destlen = 0;

    if (srclen && src) {
        *dest = (u_char *) malloc(srclen);
        if (*dest == NULL)
            return 0;
        memmove(*dest, src, srclen);
        *destlen = srclen;
    }
    return *destlen;
}                               /* end snmpv3_clone_engineID() */


/*******************************************************************-o-******
 * snmpv3_generate_engineID
 *
 * Parameters:
 *	*length
 *      
 * Returns:
 *	Pointer to copy of engineID	On Success.
 *	NULL				If malloc() or snmpv3_get_engineID()
 *						fail.
 *
 * Generates a malloced copy of our engineID.
 *
 * 'length' is set to the length of engineID  -OR-  < 0 on failure.
 */
u_char         *
snmpv3_generate_engineID(size_t * length)
{
    u_char         *newID;
    newID = (u_char *) malloc(engineIDLength);

    if (newID) {
        *length = snmpv3_get_engineID(newID, engineIDLength);
    }

    if (*length == 0) {
        SNMP_FREE(newID);
        newID = NULL;
    }

    return newID;

}                               /* end snmpv3_generate_engineID() */

/*
 * snmpv3_local_snmpEngineTime(): return the number of seconds since the
 * snmpv3 engine last incremented engine_boots 
 */
u_long
snmpv3_local_snmpEngineTime(void)
{
    struct timeval  now;

    gettimeofday(&now, NULL);
    return calculate_time_diff(&now, &snmpv3starttime) / 100;
}


/*
 * Code only for Linux systems 
 */
#if defined(IFHWADDRLEN) && defined(SIOCGIFHWADDR)
static int
getHwAddress(const char *networkDevice, /* e.g. "eth0", "eth1" */
             char *addressOut)
{                               /* return address. Len=IFHWADDRLEN */
    /*
     * getHwAddress(...)
     * *
     * *  This function will return a Network Interfaces Card's Hardware
     * *  address (aka MAC address).
     * *
     * *  Input Parameter(s):
     * *      networkDevice - a null terminated string with the name of a network
     * *                      device.  Examples: eth0, eth1, etc...
     * *
     * *  Output Parameter(s):
     * *      addressOut -    This is the binary value of the hardware address.
     * *                      This value is NOT converted into a hexadecimal string.
     * *                      The caller must pre-allocate for a return value of
     * *                      length IFHWADDRLEN
     * *
     * *  Return value:   This function will return zero (0) for success.  If
     * *                  an error occurred the function will return -1.
     * *
     * *  Caveats:    This has only been tested on Ethernet networking cards.
     */
    int             sock;       /* our socket */
    struct ifreq    request;    /* struct which will have HW address */

    if ((NULL == networkDevice) || (NULL == addressOut)) {
        return -1;
    }
    /*
     * In order to find out the hardware (MAC) address of our system under
     * * Linux we must do the following:
     * * 1.  Create a socket
     * * 2.  Do an ioctl(...) call with the SIOCGIFHWADDRLEN operation.
     */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return -1;
    }
    /*
     * erase the request block 
     */
    memset(&request, 0, sizeof(request));
    /*
     * copy the name of the net device we want to find the HW address for 
     */
    strncpy(request.ifr_name, networkDevice, IFNAMSIZ - 1);
    /*
     * Get the HW address 
     */
    if (ioctl(sock, SIOCGIFHWADDR, &request)) {
        close(sock);
        return -1;
    }
    close(sock);
    memcpy(addressOut, request.ifr_hwaddr.sa_data, IFHWADDRLEN);
    return 0;
}
#endif

#ifdef SNMP_TESTING_CODE
/*
 * snmpv3_set_engineBootsAndTime(): this function does not exist.  Go away. 
 */
/*
 * It certainly should never be used, unless in a testing scenero,
 * which is why it was created 
 */
void
snmpv3_set_engineBootsAndTime(int boots, int ttime)
{
    engineBoots = boots;
    gettimeofday(&snmpv3starttime, NULL);
    snmpv3starttime.tv_sec -= ttime;
}
#endif
