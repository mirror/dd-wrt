/*
 * snmp_parse_args.c
 */
/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright @ 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>
#include <errno.h>

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
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/library/snmpv3.h>
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
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
    fprintf(outf, "SNMP Version 1 or 2c specific\n");
    fprintf(outf, "  -c COMMUNITY\t\tset the community string\n");
#endif /* support for community based SNMP */
    fprintf(outf, "SNMP Version 3 specific\n");
    fprintf(outf,
            "  -a PROTOCOL\t\tset authentication protocol (MD5|SHA|SHA-224|SHA-256|SHA-384|SHA-512)\n");
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
    fprintf(outf, "  -x PROTOCOL\t\tset privacy protocol (DES|AES"
#ifdef NETSNMP_DRAFT_BLUMENTHAL_AES_04
            "|AES-192|AES-256"
#endif
            ")\n");
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
#ifndef NETSNMP_DISABLE_DEBUGGING
    fprintf(outf,
            "  -D[TOKEN[,...]]\tturn on debugging output for the specified TOKENs\n\t\t\t   (ALL gives extremely verbose debugging output)\n");
#endif
    fprintf(outf, "General options\n");
    fprintf(outf,
            "  -m MIB[" ENV_SEPARATOR "...]\t\tload given list of MIBs (ALL loads everything)\n");
    fprintf(outf,
            "  -M DIR[" ENV_SEPARATOR "...]\t\tlook in given list of directories for MIBs\n");
#ifndef NETSNMP_DISABLE_MIB_LOADING
    fprintf(outf,
            "    (default: %s)\n", netsnmp_get_mib_directory());
    fprintf(outf,
            "  -P MIBOPTS\t\tToggle various defaults controlling MIB parsing:\n");
    snmp_mib_toggle_options_usage("\t\t\t  ", outf);
#endif
    fprintf(outf,
            "  -O OUTOPTS\t\tToggle various defaults controlling output display:\n");
    snmp_out_toggle_options_usage("\t\t\t  ", outf);
    fprintf(outf,
            "  -I INOPTS\t\tToggle various defaults controlling input parsing:\n");
    snmp_in_toggle_options_usage("\t\t\t  ", outf);
    fprintf(outf,
            "  -L LOGOPTS\t\tToggle various defaults controlling logging:\n");
    snmp_log_options_usage("\t\t\t  ", outf);
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
    cp = (char *)malloc(strlen(myoptarg) + 3);
    if (!cp)
        return;
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

int
netsnmp_parse_args(int argc,
                   char **argv,
                   netsnmp_session * session, const char *localOpts,
                   void (*proc) (int, char *const *, int),
                   int flags)
{
    int             arg, ret, sp = 0;
    char           *cp;
    char           *Apsz = NULL;
    char           *Xpsz = NULL;
    char           *Cpsz = NULL;
    char            Opts[BUF_SIZE];
    int             zero_sensitive = !( flags & NETSNMP_PARSE_ARGS_NOZERO );
    char           *backup_NETSNMP_DS_LIB_OUTPUT_PRECISION = NULL;

    /*
     * initialize session to default values 
     */
    snmp_sess_init(session);
    strcpy(Opts, "Y:VhHm:M:O:I:P:D:dv:r:t:c:Z:e:E:n:u:l:x:X:a:A:p:T:-:3:L:s:");
    if (localOpts) {
        if (strlen(localOpts) + strlen(Opts) >= sizeof(Opts)) {
            snmp_log(LOG_ERR, "Too many localOpts in snmp_parse_args()\n");
            return -1;
        }
        strcat(Opts, localOpts);
    }

    /*
     * get the options 
     */
    DEBUGMSGTL(("snmp_parse_args", "starting: %d/%d\n", optind, argc));
    for (arg = 0; arg < argc; arg++) {
        DEBUGMSGTL(("snmp_parse_args", " arg %d = %s\n", arg, argv[arg]));
    }

    optind = 1;
    while ((arg = getopt(argc, argv, Opts)) != EOF) {
        DEBUGMSGTL(("snmp_parse_args", "handling (#%d): %c (optarg %s) (sp %d)\n",
                    optind, arg, optarg, sp));
        switch (arg) {
        case '-':
            if (strcasecmp(optarg, "help") == 0) {
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            if (strcasecmp(optarg, "version") == 0) {
                fprintf(stderr,"NET-SNMP version: %s\n",netsnmp_get_version());
                ret = NETSNMP_PARSE_ARGS_SUCCESS_EXIT;
                goto out;
            }

            handle_long_opt(optarg);
            break;

        case 'V':
            fprintf(stderr, "NET-SNMP version: %s\n", netsnmp_get_version());
            ret = NETSNMP_PARSE_ARGS_SUCCESS_EXIT;
            goto out;

        case 'h':
            ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
            goto out;

        case 'H':
            init_snmp(NETSNMP_APPLICATION_CONFIG_TYPE);
            fprintf(stderr, "Configuration directives understood:\n");
            read_config_print_usage("  ");
            ret = NETSNMP_PARSE_ARGS_SUCCESS_EXIT;
            goto out;

        case 'Y':
            netsnmp_config_remember(optarg);
            break;

#ifndef NETSNMP_DISABLE_MIB_LOADING
        case 'm':
            setenv("MIBS", optarg, 1);
            break;

        case 'M':
            netsnmp_get_mib_directory(); /* prepare the default directories */
            netsnmp_set_mib_directory(optarg);
            break;
#endif /* NETSNMP_DISABLE_MIB_LOADING */

        case 'O':
            cp = snmp_out_options(optarg, argc, argv);
            if (cp != NULL) {
                fprintf(stderr, "Unknown output option passed to -O: %c.\n", 
			*cp);
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;

        case 'I':
            cp = snmp_in_options(optarg, argc, argv);
            if (cp != NULL) {
                fprintf(stderr, "Unknown input option passed to -I: %c.\n",
			*cp);
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;

#ifndef NETSNMP_DISABLE_MIB_LOADING
        case 'P':
            cp = snmp_mib_toggle_options(optarg);
            if (cp != NULL) {
                fprintf(stderr,
                        "Unknown parsing option passed to -P: %c.\n", *cp);
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;
#endif /* NETSNMP_DISABLE_MIB_LOADING */

        case 'D':
#ifdef NETSNMP_NO_DEBUGGING
            fprintf(stderr, "Debug not configured in\n");
            ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
            goto out;
#else
            debug_register_tokens(optarg);
            snmp_set_do_debugging(1);
#endif
            break;

        case 'd':
            netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, 
				   NETSNMP_DS_LIB_DUMP_PACKET, 1);
            break;

        case 's':
            session->localname = strdup(optarg);
            break;

        case 'v':
            session->version = -1;
#ifndef NETSNMP_DISABLE_SNMPV1
            if (!strcmp(optarg, "1")) {
                session->version = SNMP_VERSION_1;
            }
#endif
#ifndef NETSNMP_DISABLE_SNMPV2C
            if (!strcasecmp(optarg, "2c")) {
                session->version = SNMP_VERSION_2c;
            }
#endif
            if (!strcasecmp(optarg, "3")) {
                session->version = SNMP_VERSION_3;
            }
            if (session->version == -1) {
                fprintf(stderr,
                        "Invalid version specified after -v flag: %s\n",
                        optarg);
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;

        case 'p':
            fprintf(stderr, "Warning: -p option is no longer used - ");
            fprintf(stderr, "specify the remote host as HOST:PORT\n");
            ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
            goto out;

        case 'T':
        {
            char leftside[SNMP_MAXBUF_MEDIUM], rightside[SNMP_MAXBUF_MEDIUM];
            char *tmpcp, *tmpopt;
            
            /* ensure we have a proper argument */
            tmpopt = strdup(optarg);
            tmpcp = strchr(tmpopt, '=');
            if (!tmpcp) {
                fprintf(stderr, "-T expects a NAME=VALUE pair.\n");
                free(tmpopt);
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            *tmpcp++ = '\0';

            /* create the transport config container if this is the first */
            if (!session->transport_configuration) {
                netsnmp_container_init_list();
                session->transport_configuration =
                    netsnmp_container_find("transport_configuration:fifo");
                if (!session->transport_configuration) {
                    fprintf(stderr, "failed to initialize the transport configuration container\n");
                    free(tmpopt);
                    ret = NETSNMP_PARSE_ARGS_ERROR;
                    goto out;
                }

                session->transport_configuration->compare =
                    (netsnmp_container_compare*)
                    netsnmp_transport_config_compare;
            }

            /* set the config */
            strlcpy(leftside, tmpopt, sizeof(leftside));
            strlcpy(rightside, tmpcp, sizeof(rightside));

            CONTAINER_INSERT(session->transport_configuration,
                             netsnmp_transport_create_config(leftside,
                                                             rightside));
            free(tmpopt);
        }
        break;
            
        case 't':
            session->timeout = (long)(atof(optarg) * 1000000L);
            if (session->timeout <= 0) {
                fprintf(stderr, "Invalid timeout in seconds after -t flag.\n");
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;

        case 'r':
            session->retries = atoi(optarg);
            if (session->retries < 0 || !isdigit((unsigned char)(optarg[0]))) {
                fprintf(stderr, "Invalid number of retries after -r flag.\n");
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;

        case 'c':
	    if (zero_sensitive) {
                SNMP_FREE(Cpsz); /* free any previous value */
		if ((Cpsz = strdup(optarg)) != NULL) {
		    memset(optarg, '\0', strlen(optarg));
		} else {
		    fprintf(stderr, "malloc failure processing -c flag.\n");
		    ret = NETSNMP_PARSE_ARGS_ERROR;
                    goto out;
		}
	    } else {
		Cpsz = strdup(optarg);
	    }
            break;

        case '3':
            if (snmpv3_parse_args(optarg, session, &Apsz, &Xpsz, argc, argv,
                                  flags) < 0){
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;

        case 'L':
            if (snmp_log_options(optarg, argc, argv) < 0) {
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;

#define SNMPV3_CMD_OPTIONS
#ifdef  SNMPV3_CMD_OPTIONS
        case 'Z':
        case 'e':
        case 'E':
        case 'n':
        case 'l':
        case 'u':
#ifdef NETSNMP_SECMOD_USM
        case 'a':
        case 'x':
        case 'A':
        case 'X':
#endif             /* NETSNMP_SECMOD_USM */
            if (snmpv3_parse_arg(arg, optarg, session, &Apsz, &Xpsz, argc,
                                 argv, flags) < 0){
                ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                goto out;
            }
            break;
#endif                          /* SNMPV3_CMD_OPTIONS */

        case '?':
            ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
            goto out;

        default:
            proc(argc, argv, arg);
            break;
        }
    }
    DEBUGMSGTL(("snmp_parse_args", "finished: %d/%d\n", optind, argc));
    
    /*
     * save command line parameters which should have precedence above config file settings
     *    (There ought to be a more scalable approach than this....)
     */
    if (netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OUTPUT_PRECISION)) {
        backup_NETSNMP_DS_LIB_OUTPUT_PRECISION = 
            strdup(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OUTPUT_PRECISION));
    }

    /*
     * read in MIB database and initialize the snmp library, read the config file
     */
    init_snmp(NETSNMP_APPLICATION_CONFIG_TYPE);

    /*
     * restore command line parameters which should have precedence above config file settings
     */
    if(backup_NETSNMP_DS_LIB_OUTPUT_PRECISION) {
        netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OUTPUT_PRECISION, backup_NETSNMP_DS_LIB_OUTPUT_PRECISION);
        free(backup_NETSNMP_DS_LIB_OUTPUT_PRECISION);
    }

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
            switch (NETSNMP_DEFAULT_SNMP_VERSION) {
#ifndef NETSNMP_DISABLE_SNMPV1
            case 1:
                session->version = SNMP_VERSION_1;
                break;
#endif

#ifndef NETSNMP_DISABLE_SNMPV2C
            case 2:
                session->version = SNMP_VERSION_2c;
                break;
#endif

            case 3:
                session->version = SNMP_VERSION_3;
                break;

            default:
                snmp_log(LOG_ERR, "Can't determine a valid SNMP version for the session\n");
                return(NETSNMP_PARSE_ARGS_ERROR);
            }
        } else {
#ifndef NETSNMP_DISABLE_SNMPV1
            if (session->version == NETSNMP_DS_SNMP_VERSION_1)  /* bogus value.  version 1 actually = 0 */
                session->version = SNMP_VERSION_1;
#endif
        }
    }

#ifdef NETSNMP_SECMOD_USM
    /* XXX: this should ideally be moved to snmpusm.c somehow */

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
            session->securityAuthProto =
                snmp_duplicate_objid(SNMP_DEFAULT_AUTH_PROTO,
                                     SNMP_DEFAULT_AUTH_PROTOLEN);
            session->securityAuthProtoLen = SNMP_DEFAULT_AUTH_PROTOLEN;
        }
        if (generate_Ku(session->securityAuthProto,
                        session->securityAuthProtoLen,
                        (u_char *) Apsz, strlen(Apsz),
                        session->securityAuthKey,
                        &session->securityAuthKeyLen) != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr,
                    "Error generating a key (Ku) from the supplied authentication pass phrase. \n");
            ret = NETSNMP_PARSE_ARGS_ERROR;
            goto out;
        }
        free(Apsz);
        Apsz = NULL;
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
            session->securityPrivProto =
                snmp_duplicate_objid(SNMP_DEFAULT_PRIV_PROTO,
                                     SNMP_DEFAULT_PRIV_PROTOLEN);
            session->securityPrivProtoLen = SNMP_DEFAULT_PRIV_PROTOLEN;
        }
        if (generate_Ku(session->securityAuthProto,
                        session->securityAuthProtoLen,
                        (u_char *) Xpsz, strlen(Xpsz),
                        session->securityPrivKey,
                        &session->securityPrivKeyLen) != SNMPERR_SUCCESS) {
            snmp_perror(argv[0]);
            fprintf(stderr,
                    "Error generating a key (Ku) from the supplied privacy pass phrase. \n");
            ret = NETSNMP_PARSE_ARGS_ERROR;
            goto out;
        }
        free(Xpsz);
        Xpsz = NULL;
    }
#endif /* NETSNMP_SECMOD_USM */

    /*
     * get the hostname 
     */
    if (optind == argc) {
        fprintf(stderr, "No hostname specified.\n");
        ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
        goto out;
    }
    session->peername = argv[optind++]; /* hostname */

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
    /*
     * If v1 or v2c, check community has been set, either by a -c option above,
     * or via a default token somewhere.  
     * If neither, it will be taken from the incoming request PDU.
     */

#if defined(NETSNMP_DISABLE_SNMPV1)
    if (session->version == SNMP_VERSION_2c)
#else 
#if defined(NETSNMP_DISABLE_SNMPV2C)
    if (session->version == SNMP_VERSION_1)
#else
    if (session->version == SNMP_VERSION_1 ||
	session->version == SNMP_VERSION_2c)
#endif
#endif
    {
        if (Cpsz == NULL) {
            Cpsz = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
					 NETSNMP_DS_LIB_COMMUNITY);
	    if (Cpsz == NULL) {
                if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                           NETSNMP_DS_LIB_IGNORE_NO_COMMUNITY)){
                    DEBUGMSGTL(("snmp_parse_args",
                                "ignoring that the community string is not present\n"));
                    session->community = NULL;
                    session->community_len = 0;
                } else {
                    fprintf(stderr, "No community name specified.\n");
                    ret = NETSNMP_PARSE_ARGS_ERROR_USAGE;
                    goto out;
                }
	    } else {
                Cpsz = NULL;
            }
	} else {
            session->community = (unsigned char *)Cpsz;
            session->community_len = strlen(Cpsz);
            Cpsz = NULL;
        }
    }
#endif /* support for community based SNMP */

    ret = optind;

out:
    free(Apsz);
    free(Xpsz);
    free(Cpsz);
    return ret;
}

int
snmp_parse_args(int argc,
                char **argv,
                netsnmp_session * session, const char *localOpts,
                void (*proc) (int, char *const *, int))
{
    return netsnmp_parse_args(argc, argv, session, localOpts, proc, 0);
}
