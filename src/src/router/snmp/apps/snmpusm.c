/*
 * snmpusm.c - send snmp SET requests to a network entity to change the
 *             usm user database
 *
 * XXX get engineID dynamicly.
 * XXX read passwords from prompts
 * XXX customize responses with user names, etc.
 */
#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <ctype.h>
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
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>

int             main(int, char **);

#define CMD_PASSWD_NAME    "passwd"
#define CMD_PASSWD         1
#define CMD_CREATE_NAME    "create"
#define CMD_CREATE         2
#define CMD_DELETE_NAME    "delete"
#define CMD_DELETE         3
#define CMD_CLONEFROM_NAME "cloneFrom"
#define CMD_CLONEFROM      4

#define CMD_NUM    4

static const char *successNotes[CMD_NUM] = {
    "SNMPv3 Key(s) successfully changed.",
    "User successfully created.",
    "User successfully deleted.",
    "User successfully cloned."
};

#define                   USM_OID_LEN    12

static oid      authKeyOid[MAX_OID_LEN] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 6 }, ownAuthKeyOid[MAX_OID_LEN] = {
1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 7}, privKeyOid[MAX_OID_LEN] = {
1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 9}, ownPrivKeyOid[MAX_OID_LEN] = {
1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 10}, usmUserCloneFrom[MAX_OID_LEN] = {
1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 4}, usmUserSecurityName[MAX_OID_LEN] = {
1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 3}, usmUserStatus[MAX_OID_LEN] = {
1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 13}

;

static
oid            *authKeyChange = authKeyOid, *privKeyChange = privKeyOid;
int             doauthkey = 0, doprivkey = 0;

void
usage(void)
{
    fprintf(stderr, "Usage: snmpusm ");
    snmp_parse_args_usage(stderr);
    fprintf(stderr, " COMMAND\n\n");
    snmp_parse_args_descriptions(stderr);
    fprintf(stderr, "\nsnmpusm commands:\n");
    fprintf(stderr, "  create    USER [CLONEFROM-USER]\n");
    fprintf(stderr, "  delete    USER\n");
    fprintf(stderr, "  cloneFrom USER CLONEFROM-USER\n");
    fprintf(stderr,
            "  passwd    [-Co] [-Ca] [-Cx] OLD-PASSPHRASE NEW-PASSPHRASE\n");
    fprintf(stderr, "\t\t-Co\t\tUse the ownKeyChange objects.\n");
    fprintf(stderr, "\t\t-Cx\t\tChange the privacy key.\n");
    fprintf(stderr, "\t\t-Ca\t\tChange the authentication key.\n");
}

/*
 * setup_oid appends to the oid the index for the engineid/user 
 */
void
setup_oid(oid * it, size_t * len, u_char * id, size_t idlen,
          const char *user)
{
    int             i, itIndex = 12;

    *len = itIndex + 1 + idlen + 1 + strlen(user);

    it[itIndex++] = idlen;
    for (i = 0; i < (int) idlen; i++) {
        it[itIndex++] = id[i];
    }

    it[itIndex++] = strlen(user);
    for (i = 0; i < (int) strlen(user); i++) {
        it[itIndex++] = user[i];
    }

    /*
     * fprintf(stdout, "setup_oid: ");  
     */
    /*
     * fprint_objid(stdout, it, *len);  
     */
    /*
     * fprintf(stdout, "\n");  
     */
}

static void
optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'o':
                authKeyChange = ownAuthKeyOid;
                privKeyChange = ownPrivKeyOid;
                break;

            case 'a':
                doauthkey = 1;
                break;

            case 'x':
                doprivkey = 1;
                break;

            default:
                fprintf(stderr, "Unknown flag passed to -C: %c\n",
                        optarg[-1]);
                exit(1);
            }
        }
        break;
    }
}

int
main(int argc, char *argv[])
{
    netsnmp_session session, *ss;
    netsnmp_pdu    *pdu = NULL, *response = NULL;
#ifdef notused
    netsnmp_variable_list *vars;
#endif

    int             arg;
#ifdef notused
    int             count;
    int             current_name = 0;
    int             current_type = 0;
    int             current_value = 0;
    char           *names[128];
    char            types[128];
    char           *values[128];
    oid             name[MAX_OID_LEN];
#endif
    size_t          name_length = USM_OID_LEN;
    size_t          name_length2 = USM_OID_LEN;
    int             status;
    int             exitval = 0;
    int             rval;
    int             command = 0;
    long            longvar;

    size_t          oldKu_len = SNMP_MAXBUF_SMALL,
        newKu_len = SNMP_MAXBUF_SMALL,
        oldkul_len = SNMP_MAXBUF_SMALL,
        newkul_len = SNMP_MAXBUF_SMALL, keychange_len = SNMP_MAXBUF_SMALL;

    char           *newpass = NULL, *oldpass = NULL;
    u_char          oldKu[SNMP_MAXBUF_SMALL],
        newKu[SNMP_MAXBUF_SMALL],
        oldkul[SNMP_MAXBUF_SMALL],
        newkul[SNMP_MAXBUF_SMALL], keychange[SNMP_MAXBUF_SMALL];

    authKeyChange = authKeyOid;
    privKeyChange = privKeyOid;

    /*
     * get the common command line arguments 
     */
    switch (arg = snmp_parse_args(argc, argv, &session, "C:", optProc)) {
    case -2:
        exit(0);
    case -1:
        usage();
        exit(1);
    default:
        break;
    }

    SOCK_STARTUP;

    /*
     * open an SNMP session 
     */
    /*
     * Note:  this wil obtain the engineID needed below 
     */
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
        snmp_sess_perror("snmpusm", &session);
        exit(1);
    }

    /*
     * create PDU for SET request and add object names and values to request 
     */
    pdu = snmp_pdu_create(SNMP_MSG_SET);

    if (arg >= argc) {
        fprintf(stderr, "Please specify a opreation to perform.\n");
        usage();
        exit(1);
    }

    if (strcmp(argv[arg], CMD_PASSWD_NAME) == 0) {

        /*
         * passwd: change a users password.
         *
         * XXX:  Uses the auth type of the calling user, a MD5 user can't
         *       change a SHA user's key.
         */
        command = CMD_PASSWD;
        oldpass = argv[++arg];
        newpass = argv[++arg];

        if (doprivkey == 0 && doauthkey == 0)
            doprivkey = doauthkey = 1;

        if (newpass == NULL || strlen(newpass) < USM_LENGTH_P_MIN) {
            fprintf(stderr,
                    "New passphrase must be greater than %d characters in length.\n",
                    USM_LENGTH_P_MIN);
            exit(1);
        }

        if (oldpass == NULL || strlen(oldpass) < USM_LENGTH_P_MIN) {
            fprintf(stderr,
                    "Old passphrase must be greater than %d characters in length.\n",
                    USM_LENGTH_P_MIN);
            exit(1);
        }

        /*
         * do we have a securityName?  If not, copy the default 
         */
        if (session.securityName == NULL) {
            session.securityName = 
	      strdup(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
					   NETSNMP_DS_LIB_SECNAME));
        }

        /*
         * the old Ku is in the session, but we need the new one 
         */
        if (session.securityAuthProto == NULL) {
            /*
             * get .conf set default 
             */
            const oid      *def =
                get_default_authtype(&session.securityAuthProtoLen);
            session.securityAuthProto =
                snmp_duplicate_objid(def, session.securityAuthProtoLen);
        }
        if (session.securityAuthProto == NULL) {
            /*
             * assume MD5 
             */
            session.securityAuthProtoLen =
                sizeof(usmHMACMD5AuthProtocol) / sizeof(oid);
            session.securityAuthProto =
                snmp_duplicate_objid(usmHMACMD5AuthProtocol,
                                     session.securityAuthProtoLen);
        }
        rval = generate_Ku(session.securityAuthProto,
                           session.securityAuthProtoLen,
                           (u_char *) newpass, strlen(newpass),
                           newKu, &newKu_len);

        if (rval != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr, "generating the old Ku failed\n");
            exit(1);
        }

        /*
         * the old Ku is in the session, but we need the new one 
         */
        rval = generate_Ku(session.securityAuthProto,
                           session.securityAuthProtoLen,
                           (u_char *) oldpass, strlen(oldpass),
                           oldKu, &oldKu_len);

        if (rval != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr, "generating the new Ku failed\n");
            exit(1);
        }

        /*
         * generate the two Kul's 
         */
        rval = generate_kul(session.securityAuthProto,
                            session.securityAuthProtoLen,
                            ss->contextEngineID, ss->contextEngineIDLen,
                            oldKu, oldKu_len, oldkul, &oldkul_len);

        if (rval != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr, "generating the old Kul failed\n");
            exit(1);
        }

        rval = generate_kul(session.securityAuthProto,
                            session.securityAuthProtoLen,
                            ss->contextEngineID, ss->contextEngineIDLen,
                            newKu, newKu_len, newkul, &newkul_len);

        if (rval != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr, "generating the new Kul failed\n");
            exit(1);
        }

        /*
         * create the keychange string 
         */
        rval = encode_keychange(session.securityAuthProto,
                                session.securityAuthProtoLen,
                                oldkul, oldkul_len,
                                newkul, newkul_len,
                                keychange, &keychange_len);

        if (rval != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr, "encoding the keychange failed\n");
            usage();
            exit(1);
        }

        /*
         * add the keychange string to the outgoing packet 
         */
        if (doauthkey) {
            setup_oid(authKeyChange, &name_length,
                      ss->contextEngineID, ss->contextEngineIDLen,
                      session.securityName);
            snmp_pdu_add_variable(pdu, authKeyChange, name_length,
                                  ASN_OCTET_STR, keychange, keychange_len);
        }
        if (doprivkey) {
            setup_oid(privKeyChange, &name_length,
                      ss->contextEngineID, ss->contextEngineIDLen,
                      session.securityName);
            snmp_pdu_add_variable(pdu, privKeyChange, name_length,
                                  ASN_OCTET_STR, keychange, keychange_len);
        }

    } else if (strcmp(argv[arg], CMD_CREATE_NAME) == 0) {
        /*
         * create:  create a user
         *
         * create USER [CLONEFROM]
         */
        if (++arg >= argc) {
            fprintf(stderr, "You must specify the user name to create\n");
            usage();
            exit(1);
        }

        command = CMD_CREATE;
        setup_oid(usmUserStatus, &name_length,
                  ss->contextEngineID, ss->contextEngineIDLen, argv[arg]);
        longvar = RS_CREATEANDGO;
        snmp_pdu_add_variable(pdu, usmUserStatus, name_length,
                              ASN_INTEGER, (u_char *) & longvar,
                              sizeof(longvar));

        if (++arg < argc) {
            /*
             * clone the new user from another user as well 
             */
            setup_oid(usmUserCloneFrom, &name_length,
                      ss->contextEngineID, ss->contextEngineIDLen,
                      argv[arg - 1]);
            setup_oid(usmUserSecurityName, &name_length2,
                      ss->contextEngineID, ss->contextEngineIDLen,
                      argv[arg]);
            snmp_pdu_add_variable(pdu, usmUserCloneFrom, name_length,
                                  ASN_OBJECT_ID,
                                  (u_char *) usmUserSecurityName,
                                  sizeof(oid) * name_length2);
        }

    } else if (strcmp(argv[arg], CMD_CLONEFROM_NAME) == 0) {
        /*
         * create:  clone a user from another
         *
         * cloneFrom USER FROM
         */
        if (++arg >= argc) {
            fprintf(stderr,
                    "You must specify the user name to operate on\n");
            usage();
            exit(1);
        }

        command = CMD_CLONEFROM;
        setup_oid(usmUserCloneFrom, &name_length,
                  ss->contextEngineID, ss->contextEngineIDLen, argv[arg]);

        if (++arg >= argc) {
            fprintf(stderr,
                    "You must specify the user name to clone from\n");
            usage();
            exit(1);
        }

        setup_oid(usmUserSecurityName, &name_length2,
                  ss->contextEngineID, ss->contextEngineIDLen, argv[arg]);
        snmp_pdu_add_variable(pdu, usmUserCloneFrom, name_length,
                              ASN_OBJECT_ID,
                              (u_char *) usmUserSecurityName,
                              sizeof(oid) * name_length2);

    } else if (strcmp(argv[arg], CMD_DELETE_NAME) == 0) {
        /*
         * delete:  delete a user
         *
         * delete USER
         */
        if (++arg >= argc) {
            fprintf(stderr, "You must specify the user name to delete\n");
            exit(1);
        }

        command = CMD_DELETE;
        setup_oid(usmUserStatus, &name_length,
                  ss->contextEngineID, ss->contextEngineIDLen, argv[arg]);
        longvar = RS_DESTROY;
        snmp_pdu_add_variable(pdu, usmUserStatus, name_length,
                              ASN_INTEGER, (u_char *) & longvar,
                              sizeof(longvar));
    } else {
        fprintf(stderr, "Unknown command\n");
        usage();
        exit(1);
    }


    /*
     * do the request 
     */
    status = snmp_synch_response(ss, pdu, &response);
    if (status == STAT_SUCCESS) {
        if (response) {
            if (response->errstat == SNMP_ERR_NOERROR) {
                fprintf(stderr, "%s\n", successNotes[command - 1]);
            } else {
                fprintf(stderr, "Error in packet.\nReason: %s\n",
                        snmp_errstring(response->errstat));
                if (response->errindex != 0) {
                    int             count;
                    netsnmp_variable_list *vars;
                    fprintf(stderr, "Failed object: ");
                    for (count = 1, vars = response->variables;
                         vars && count != response->errindex;
                         vars = vars->next_variable, count++)
                        /*EMPTY*/;
                    if (vars)
                        fprint_objid(stderr, vars->name,
                                     vars->name_length);
                    fprintf(stderr, "\n");
                }
                exitval = 2;
            }
        }
    } else if (status == STAT_TIMEOUT) {
        fprintf(stderr, "Timeout: No Response from %s\n",
                session.peername);
        exitval = 1;
    } else {                    /* status == STAT_ERROR */
        snmp_sess_perror("snmpset", ss);
        exitval = 1;
    }

    if (response)
        snmp_free_pdu(response);
    snmp_close(ss);
    SOCK_CLEANUP;
    return exitval;
}
