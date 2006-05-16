/*
 * snmp_parse_args.c
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
#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
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

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/library/snmp_parse_args.h>   /* for "internal" definitions */
#include <net-snmp/utilities.h>

#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/snmp_client.h>
#include <net-snmp/library/mib.h>
#include <net-snmp/library/scapi.h>
#include <net-snmp/library/keytools.h>

#include <net-snmp/version.h>
#include <net-snmp/library/parse.h>
#include <net-snmp/library/snmpv3.h>
#include <net-snmp/library/transform_oids.h>

int             random_access = 0;

void
snmp_parse_args_usage(FILE * outf)
{
    fprintf(outf, "[OPTIONS] AGENT");
}

void
snmp_parse_args_descriptions(FILE * outf)
{
    fprintf(outf, "  Version:  %s\n", netsnmp_get_version());
    fprintf(outf, "  Web:      http://www.net-snmp.org/\n");
    fprintf(outf,
            "  Email:    net-snmp-coders@lists.sourceforge.net\n\nOPTIONS:\n");
    fprintf(outf, "  -h, --help\t\tdisplay this help message\n");
    fprintf(outf,
            "  -H\t\t\tdisplay configuration file directives understood\n");
    fprintf(outf, "  -v 1|2c|3\t\tspecifies SNMP version to use\n");
    fprintf(outf, "  -V, --version\t\tdisplay package version number\n");
    fprintf(outf, "SNMP Version 1 or 2c specific\n");
    fprintf(outf, "  -c COMMUNITY\t\tset the community string\n");
    fprintf(outf, "SNMP Version 3 specific\n");
    fprintf(outf,
            "  -a PROTOCOL\t\tset authentication protocol (MD5|SHA)\n");
    fprintf(outf,
            "  -A PASSPHRASE\t\tset authentication protocol pass phrase\n");
    fprintf(outf,
            "  -e ENGINE-ID\t\tset security engine ID (e.g. 800000020109840301)\n");
    fprintf(outf,
            "  -E ENGINE-ID\t\tset context engine ID (e.g. 800000020109840301)\n");
    fprintf(outf,
            "  -l LEVEL\t\tset security level (noAuthNoPriv|authNoPriv|authPriv)\n");
    fprintf(outf, "  -n CONTEXT\t\tset context name (e.g. bridge1)\n");
    fprintf(outf, "  -u USER-NAME\t\tset security name (e.g. bert)\n");
#ifdef HAVE_AES
    fprintf(outf, "  -x PROTOCOL\t\tset privacy protocol (DES|AES)\n");
#else
    fprintf(outf, "  -x PROTOCOL\t\tset privacy protocol (DES)\n");
#endif
    fprintf(outf, "  -X PASSPHRASE\t\tset privacy protocol pass phrase\n");
    fprintf(outf,
            "  -Z BOOTS,TIME\t\tset destination engine boots/time\n");
    fprintf(outf, "General communication options\n");
    fprintf(outf, "  -r RETRIES\t\tset the number of retries\n");
    fprintf(outf,
            "  -t TIMEOUT\t\tset the request timeout (in seconds)\n");
    fprintf(outf, "Debugging\n");
    fprintf(outf, "  -d\t\t\tdump input/output packets in hexadecimal\n");
    fprintf(outf,
            "  -D TOKEN[,...]\tturn on debugging output for the specified TOKENs\n\t\t\t   (ALL gives extremely verbose debugging output)\n");
    fprintf(outf, "General options\n");
    fprintf(outf,
            "  -m MIB[:...]\t\tload given list of MIBs (ALL loads everything)\n");
    fprintf(outf,
            "  -M DIR[:...]\t\tlook in given list of directories for MIBs\n");
    fprintf(outf,
            "  -P MIBOPTS\t\tToggle various defaults controlling MIB parsing:\n");
    snmp_mib_toggle_options_usage("\t\t\t  ", outf);
    fprintf(outf,
            "  -O OUTOPTS\t\tToggle various defaults controlling output display:\n");
    snmp_out_toggle_options_usage("\t\t\t  ", outf);
    fprintf(outf,
            "  -I INOPTS\t\tToggle various defaults controlling input parsing:\n");
    snmp_in_toggle_options_usage("\t\t\t  ", outf);
    fflush(outf);
}

#define BUF_SIZE 512

void
handle_long_opt(const char *myoptarg)
{
    char           *cp, *cp2;
    /*
     * else it's a long option, so process it like name=value 
     */
    cp = malloc(strlen(myoptarg) + 3);
    strcpy(cp, myoptarg);
    cp2 = strchr(cp, '=');
    if (!cp2 && !strchr(cp, ' ')) {
        /*
         * well, they didn't specify an argument as far as we
         * can tell.  Give them a '1' as the argument (which
         * works for boolean tokens and a few others) and let
         * them suffer from there if it's not what they
         * wanted 
         */
        strcat(cp, " 1");
    } else {
        /*
         * replace the '=' with a ' ' 
         */
        if (cp2)
            *cp2 = ' ';
    }
    netsnmp_config(cp);
    free(cp);
}

extern int      snmpv3_options(char *optarg, netsnmp_session * session,
                               char **Apsz, char **Xpsz, int argc,
                               char *const *argv);
int
snmp_parse_args(int argc,
                char *const *argv,
                netsnmp_session * session, const char *localOpts,
                void (*proc) (int, char *const *, int))
{
    int             arg;
    char           *cp;
    char           *Apsz = NULL;
    char           *Xpsz = NULL;
    char           *Cpsz = NULL;
    char            Opts[BUF_SIZE];

    /*
     * initialize session to default values 
     */
    snmp_sess_init(session);
    strcpy(Opts, "Y:VhHm:M:O:I:P:D:dv:r:t:c:Z:e:E:n:u:l:x:X:a:A:p:T:-:3:");
    if (localOpts)
        strcat(Opts, localOpts);

    /*
     * get the options 
     */
    DEBUGMSGTL(("snmp_parse_args", "starting: %d/%d\n", optind, argc));
    for (arg = 0; arg < argc; arg++) {
        DEBUGMSGTL(("snmp_parse_args", " arg %d = %s\n", arg, argv[arg]));
    }

    optind = 1;
    while ((arg = getopt(argc, argv, Opts)) != EOF) {
        DEBUGMSGTL(("snmp_parse_args", "handling (#%d): %c\n", optind,
                    arg));
        switch (arg) {
        case '-':
            if (strcasecmp(optarg, "help") == 0) {
                return (-1);
            }
            if (strcasecmp(optarg, "version") == 0) {
                fprintf(stderr, "NET-SNMP version: %s\n",
                        netsnmp_get_version());
                return (-2);
            }

            handle_long_opt(optarg);
            break;

        case 'V':
            fprintf(stderr, "NET-SNMP version: %s\n",
                    netsnmp_get_version());
            return (-2);

        case 'h':
            return (-1);
            break;

        case 'H':
            init_snmp("snmpapp");
            fprintf(stderr, "Configuration directives understood:\n");
            read_config_print_usage("  ");
            return (-2);

        case 'Y':
            netsnmp_config_remember(optarg);
            break;

        case 'm':
            setenv("MIBS", optarg, 1);
            break;

        case 'M':
            netsnmp_set_mib_directory(optarg);
            break;

        case 'O':
            cp = snmp_out_toggle_options(optarg);
            if (cp != NULL) {
                fprintf(stderr,
                        "Unknown output option passed to -O: %c.\n", *cp);
                return (-1);
            }
            break;

        case 'I':
            cp = snmp_in_toggle_options(optarg);
            if (cp != NULL) {
                fprintf(stderr, "Unknown input option passed to -I: %c.\n",
                        *cp);
                return (-1);
            }
            break;

        case 'P':
            cp = snmp_mib_toggle_options(optarg);
            if (cp != NULL) {
                fprintf(stderr,
                        "Unknown parsing option passed to -P: %c.\n", *cp);
                return (-1);
            }
            break;

        case 'D':
            debug_register_tokens(optarg);
            snmp_set_do_debugging(1);
            break;

        case 'd':
            netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, 
				   NETSNMP_DS_LIB_DUMP_PACKET, 1);
            break;

        case 'v':
            if (!strcmp(optarg, "1")) {
                session->version = SNMP_VERSION_1;
            } else if (!strcasecmp(optarg, "2c")) {
                session->version = SNMP_VERSION_2c;
            } else if (!strcasecmp(optarg, "3")) {
                session->version = SNMP_VERSION_3;
            } else {
                fprintf(stderr,
                        "Invalid version specified after -v flag: %s\n",
                        optarg);
                return (-1);
            }
            break;

        case 'p':
            fprintf(stderr, "Warning: -p option is no longer used - ");
            fprintf(stderr, "specify the remote host as HOST:PORT\n");
            return (-1);
            break;

        case 'T':
            fprintf(stderr, "Warning: -T option is no longer used - ");
            fprintf(stderr, "specify the remote host as TRANSPORT:HOST\n");
            return (-1);
            break;

        case 't':
            session->timeout = atoi(optarg) * 1000000L;
            if (session->timeout < 0 || !isdigit(optarg[0])) {
                fprintf(stderr,
                        "Invalid timeout in seconds after -t flag.\n");
                return (-1);
            }
            break;

        case 'r':
            session->retries = atoi(optarg);
            if (session->retries < 0 || !isdigit(optarg[0])) {
                fprintf(stderr,
                        "Invalid number of retries after -r flag.\n");
                return (-1);
            }
            break;

        case 'c':
            Cpsz = optarg;
            break;

        case '3':
            if (snmpv3_options(optarg, session, &Apsz, &Xpsz, argc, argv) <
                0) {
                return (-1);
            }
            break;

#define SNMPV3_CMD_OPTIONS
#ifdef  SNMPV3_CMD_OPTIONS
        case 'Z':
            session->engineBoots = strtoul(optarg, NULL, 10);
            if (session->engineBoots == 0 || !isdigit(optarg[0])) {
                fprintf(stderr,
                        "Need engine boots value after -Z flag.\n");
                return (-1);
            }
            cp = strchr(optarg, ',');
            if (cp && *(++cp) && isdigit(*cp))
                session->engineTime = strtoul(cp, NULL, 10);
            /*
             * Handle previous '-Z boot time' syntax 
             */
            else if ((optind < argc) && isdigit(argv[optind][0]))
                session->engineTime = strtoul(argv[optind], NULL, 10);
            else {
                fprintf(stderr, "Need engine time value after -Z flag.\n");
                return (-1);
            }
            break;

        case 'e':{
                size_t          ebuf_len = 32, eout_len = 0;
                u_char         *ebuf = (u_char *) malloc(ebuf_len);

                if (ebuf == NULL) {
                    fprintf(stderr,
                            "malloc failure processing -e flag.\n");
                    return (-1);
                }
                if (!snmp_hex_to_binary
                    (&ebuf, &ebuf_len, &eout_len, 1, optarg)) {
                    fprintf(stderr,
                            "Bad engine ID value after -e flag.\n");
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
                    fprintf(stderr,
                            "malloc failure processing -E flag.\n");
                    return (-1);
                }
                if (!snmp_hex_to_binary
                    (&ebuf, &ebuf_len, &eout_len, 1, optarg)) {
                    fprintf(stderr,
                            "Bad engine ID value after -E flag.\n");
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
            if (!strcasecmp(optarg, "noAuthNoPriv") || !strcmp(optarg, "1")
                || !strcasecmp(optarg, "nanp")) {
                session->securityLevel = SNMP_SEC_LEVEL_NOAUTH;
            } else if (!strcasecmp(optarg, "authNoPriv")
                       || !strcmp(optarg, "2")
                       || !strcasecmp(optarg, "anp")) {
                session->securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
            } else if (!strcasecmp(optarg, "authPriv")
                       || !strcmp(optarg, "3")
                       || !strcasecmp(optarg, "ap")) {
                session->securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
            } else {
                fprintf(stderr,
                        "Invalid security level specified after -l flag: %s\n",
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
                        "Invalid authentication protocol specified after -a flag: %s\n",
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
                        "Invalid privacy protocol specified after -x flag: %s\n",
                        optarg);
                return (-1);
            }
            break;

        case 'A':
            Apsz = optarg;
            break;

        case 'X':
            Xpsz = optarg;
            break;
#endif                          /* SNMPV3_CMD_OPTIONS */

        case '?':
            return (-1);
            break;

        default:
            proc(argc, argv, arg);
            break;
        }
    }
    DEBUGMSGTL(("snmp_parse_args", "finished: %d/%d\n", optind, argc));

    /*
     * read in MIB database and initialize the snmp library
     */
    init_snmp("snmpapp");

    /*
     * session default version 
     */
    if (session->version == SNMP_DEFAULT_VERSION) {
        /*
         * run time default version 
         */
        session->version = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
					      NETSNMP_DS_LIB_SNMPVERSION);

        /*
         * compile time default version 
         */
        if (!session->version) {
            switch (DEFAULT_SNMP_VERSION) {
            case 1:
                session->version = SNMP_VERSION_1;
                break;

            case 2:
                session->version = SNMP_VERSION_2c;
                break;

            case 3:
                session->version = SNMP_VERSION_3;
                break;
            }
        } else {
            if (session->version == NETSNMP_DS_SNMP_VERSION_1)  /* bogus value.  version 1 actually = 0 */
                session->version = SNMP_VERSION_1;
        }
    }

    /*
     * make master key from pass phrases 
     */
    if (Apsz) {
        session->securityAuthKeyLen = USM_AUTH_KU_LEN;
        if (session->securityAuthProto == NULL) {
            /*
             * get .conf set default 
             */
            const oid      *def =
                get_default_authtype(&session->securityAuthProtoLen);
            session->securityAuthProto =
                snmp_duplicate_objid(def, session->securityAuthProtoLen);
        }
        if (session->securityAuthProto == NULL) {
            /*
             * assume MD5 
             */
            session->securityAuthProto =
                snmp_duplicate_objid(usmHMACMD5AuthProtocol,
                                     USM_AUTH_PROTO_MD5_LEN);
            session->securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
        }
        if (generate_Ku(session->securityAuthProto,
                        session->securityAuthProtoLen,
                        (u_char *) Apsz, strlen(Apsz),
                        session->securityAuthKey,
                        &session->securityAuthKeyLen) != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr,
                    "Error generating a key (Ku) from the supplied authentication pass phrase. \n");
            return (-2);
        }
    }
    if (Xpsz) {
        session->securityPrivKeyLen = USM_PRIV_KU_LEN;
        if (session->securityPrivProto == NULL) {
            /*
             * get .conf set default 
             */
            const oid      *def =
                get_default_privtype(&session->securityPrivProtoLen);
            session->securityPrivProto =
                snmp_duplicate_objid(def, session->securityPrivProtoLen);
        }
        if (session->securityPrivProto == NULL) {
            /*
             * assume DES 
             */
            session->securityPrivProto =
                snmp_duplicate_objid(usmDESPrivProtocol,
                                     USM_PRIV_PROTO_DES_LEN);
            session->securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
        }
        if (generate_Ku(session->securityAuthProto,
                        session->securityAuthProtoLen,
                        (u_char *) Xpsz, strlen(Xpsz),
                        session->securityPrivKey,
                        &session->securityPrivKeyLen) != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr,
                    "Error generating a key (Ku) from the supplied privacy pass phrase. \n");
            return (-2);
        }
    }
    /*
     * get the hostname 
     */
    if (optind == argc) {
        fprintf(stderr, "No hostname specified.\n");
        return (-1);
    }
    session->peername = argv[optind++]; /* hostname */

    /*
     * If v1 or v2c, check community has been set, either by a -c option above,
     * or via a default token somewhere.  
     */

    if (session->version == SNMP_VERSION_1 ||
        session->version == SNMP_VERSION_2c) {
        if (Cpsz == NULL) {
            Cpsz = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
					 NETSNMP_DS_LIB_COMMUNITY);
        }
        if (Cpsz == NULL) {
            fprintf(stderr, "No community name specified.\n");
            return (-1);
        }
        session->community = (unsigned char *) Cpsz;
        session->community_len = strlen(Cpsz);
    }
    return optind;
}
