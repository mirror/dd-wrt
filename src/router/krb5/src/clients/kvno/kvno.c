/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Copyright (C) 1998 by the FundsXpress, INC.
 *
 * All rights reserved.
 *
 * Export of this software from the United States of America may require
 * a specific license from the United States Government.  It is the
 * responsibility of any person or organization contemplating export to
 * obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of FundsXpress. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  FundsXpress makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "k5-platform.h"
#include <locale.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

static char *prog;
static int quiet = 0;

static void
xusage()
{
    fprintf(stderr, _("usage: %s [-C] [-u] [-c ccache] [-e etype]\n"), prog);
    fprintf(stderr, _("\t[-k keytab] [-S sname] [-U for_user [-P]]\n"));
    fprintf(stderr, _("\t[--u2u ccache] service1 service2 ...\n"));
    exit(1);
}

static void do_v5_kvno(int argc, char *argv[], char *ccachestr, char *etypestr,
                       char *keytab_name, char *sname, int canon, int unknown,
                       char *for_user, int proxy, const char *u2u_ccname);

#include <com_err.h>
static void extended_com_err_fn(const char *myprog, errcode_t code,
                                const char *fmt, va_list args);

int
main(int argc, char *argv[])
{
    enum { OPTION_U2U = 256 };
    struct option lopts[] = {
        { "u2u", 1, NULL, OPTION_U2U },
        { NULL, 0, NULL, 0 }
    };
    const char *shopts = "uCc:e:hk:qPS:U:";
    int option;
    char *etypestr = NULL, *ccachestr = NULL, *keytab_name = NULL;
    char *sname = NULL, *for_user = NULL, *u2u_ccname = NULL;
    int canon = 0, unknown = 0, proxy = 0;

    setlocale(LC_ALL, "");
    set_com_err_hook(extended_com_err_fn);

    prog = strrchr(argv[0], '/');
    prog = prog ? (prog + 1) : argv[0];

    while ((option = getopt_long(argc, argv, shopts, lopts, NULL)) != -1) {
        switch (option) {
        case 'C':
            canon = 1;
            break;
        case 'c':
            ccachestr = optarg;
            break;
        case 'e':
            etypestr = optarg;
            break;
        case 'h':
            xusage();
            break;
        case 'k':
            keytab_name = optarg;
            break;
        case 'q':
            quiet = 1;
            break;
        case 'P':
            proxy = 1; /* S4U2Proxy - constrained delegation */
            break;
        case 'S':
            sname = optarg;
            if (unknown == 1) {
                fprintf(stderr,
                        _("Options -u and -S are mutually exclusive\n"));
                xusage();
            }
            break;
        case 'u':
            unknown = 1;
            if (sname != NULL) {
                fprintf(stderr,
                        _("Options -u and -S are mutually exclusive\n"));
                xusage();
            }
            break;
        case 'U':
            for_user = optarg; /* S4U2Self - protocol transition */
            break;
        case OPTION_U2U:
            u2u_ccname = optarg;
            break;
        default:
            xusage();
            break;
        }
    }

    if (u2u_ccname != NULL && for_user != NULL) {
        fprintf(stderr, _("Options --u2u and -P are mutually exclusive\n"));
        xusage();
    }

    if (proxy) {
        if (keytab_name == NULL) {
            fprintf(stderr, _("Option -P (constrained delegation) "
                              "requires keytab to be specified\n"));
            xusage();
        } else if (for_user == NULL) {
            fprintf(stderr, _("Option -P (constrained delegation) requires "
                              "option -U (protocol transition)\n"));
            xusage();
        }
    }

    if (argc - optind < 1)
        xusage();

    do_v5_kvno(argc - optind, argv + optind, ccachestr, etypestr, keytab_name,
               sname, canon, unknown, for_user, proxy, u2u_ccname);
    return 0;
}

#include <k5-int.h>
static krb5_context context;
static void extended_com_err_fn(const char *myprog, errcode_t code,
                                const char *fmt, va_list args)
{
    const char *emsg;

    emsg = krb5_get_error_message(context, code);
    fprintf(stderr, "%s: %s ", myprog, emsg);
    krb5_free_error_message(context, emsg);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

/* Request a single service ticket and display its status (unless quiet is
 * set).  On failure, display an error message and return non-zero. */
static krb5_error_code
kvno(const char *name, krb5_ccache ccache, krb5_principal me,
     krb5_enctype etype, krb5_keytab keytab, const char *sname,
     krb5_flags options, int unknown, krb5_principal for_user_princ, int proxy,
     krb5_data *u2u_ticket)
{
    krb5_error_code ret;
    krb5_principal server = NULL;
    krb5_ticket *ticket = NULL;
    krb5_creds in_creds, *out_creds = NULL;
    char *princ = NULL;

    memset(&in_creds, 0, sizeof(in_creds));

    if (sname != NULL) {
        ret = krb5_sname_to_principal(context, name, sname, KRB5_NT_SRV_HST,
                                      &server);
    } else {
        ret = krb5_parse_name(context, name, &server);
    }
    if (ret) {
        if (!quiet)
            com_err(prog, ret, _("while parsing principal name %s"), name);
        goto cleanup;
    }
    if (unknown)
        krb5_princ_type(context, server) = KRB5_NT_UNKNOWN;

    ret = krb5_unparse_name(context, server, &princ);
    if (ret) {
        com_err(prog, ret, _("while formatting parsed principal name for "
                             "'%s'"), name);
        goto cleanup;
    }

    in_creds.keyblock.enctype = etype;

    if (u2u_ticket != NULL)
        in_creds.second_ticket = *u2u_ticket;

    if (for_user_princ != NULL) {
        if (!proxy && !krb5_principal_compare(context, me, server)) {
            ret = EINVAL;
            com_err(prog, ret,
                    _("client and server principal names must match"));
            goto cleanup;
        }

        in_creds.client = for_user_princ;
        in_creds.server = me;
        ret = krb5_get_credentials_for_user(context, options, ccache,
                                            &in_creds, NULL, &out_creds);
    } else {
        in_creds.client = me;
        in_creds.server = server;
        ret = krb5_get_credentials(context, options, ccache, &in_creds,
                                   &out_creds);
    }

    if (ret) {
        com_err(prog, ret, _("while getting credentials for %s"), princ);
        goto cleanup;
    }

    /* We need a native ticket. */
    ret = krb5_decode_ticket(&out_creds->ticket, &ticket);
    if (ret) {
        com_err(prog, ret, _("while decoding ticket for %s"), princ);
        goto cleanup;
    }

    if (keytab != NULL) {
        ret = krb5_server_decrypt_ticket_keytab(context, keytab, ticket);
        if (ret) {
            if (!quiet) {
                fprintf(stderr, "%s: kvno = %d, keytab entry invalid\n", princ,
                        ticket->enc_part.kvno);
            }
            com_err(prog, ret, _("while decrypting ticket for %s"), princ);
            goto cleanup;
        }
        if (!quiet) {
            printf(_("%s: kvno = %d, keytab entry valid\n"), princ,
                   ticket->enc_part.kvno);
        }
        if (proxy) {
            krb5_free_creds(context, out_creds);
            out_creds = NULL;

            in_creds.client = ticket->enc_part2->client;
            in_creds.server = server;

            ret = krb5_get_credentials_for_proxy(context, KRB5_GC_CANONICALIZE,
                                                 ccache, &in_creds, ticket,
                                                 &out_creds);
            if (ret) {
                com_err(prog, ret, _("%s: constrained delegation failed"),
                        princ);
                goto cleanup;
            }
        }
    } else {
        if (!quiet)
            printf(_("%s: kvno = %d\n"), princ, ticket->enc_part.kvno);
    }

cleanup:
    krb5_free_principal(context, server);
    krb5_free_ticket(context, ticket);
    krb5_free_creds(context, out_creds);
    krb5_free_unparsed_name(context, princ);
    return ret;
}

/* Fetch the encoded local TGT for ccname's default client principal. */
static krb5_error_code
get_u2u_ticket(const char *ccname, krb5_data **ticket_out)
{
    krb5_error_code ret;
    krb5_ccache cc = NULL;
    krb5_creds mcred, *creds = NULL;

    *ticket_out = NULL;
    memset(&mcred, 0, sizeof(mcred));

    ret = krb5_cc_resolve(context, ccname, &cc);
    if (ret)
        goto cleanup;
    ret = krb5_cc_get_principal(context, cc, &mcred.client);
    if (ret)
        goto cleanup;
    ret = krb5_build_principal_ext(context, &mcred.server,
                                   mcred.client->realm.length,
                                   mcred.client->realm.data,
                                   KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME,
                                   mcred.client->realm.length,
                                   mcred.client->realm.data, 0);
    if (ret)
        goto cleanup;
    ret = krb5_get_credentials(context, KRB5_GC_CACHED, cc, &mcred, &creds);
    if (ret)
        goto cleanup;

    ret = krb5_copy_data(context, &creds->ticket, ticket_out);

cleanup:
    if (cc != NULL)
        krb5_cc_close(context, cc);
    krb5_free_cred_contents(context, &mcred);
    krb5_free_creds(context, creds);
    return ret;
}

static void
do_v5_kvno(int count, char *names[], char * ccachestr, char *etypestr,
           char *keytab_name, char *sname, int canon, int unknown,
           char *for_user, int proxy, const char *u2u_ccname)
{
    krb5_error_code ret;
    int i, errors;
    krb5_enctype etype;
    krb5_ccache ccache;
    krb5_principal me;
    krb5_keytab keytab = NULL;
    krb5_principal for_user_princ = NULL;
    krb5_flags options = canon ? KRB5_GC_CANONICALIZE : 0;
    krb5_data *u2u_ticket = NULL;

    ret = krb5_init_context(&context);
    if (ret) {
        com_err(prog, ret, _("while initializing krb5 library"));
        exit(1);
    }

    if (etypestr) {
        ret = krb5_string_to_enctype(etypestr, &etype);
        if (ret) {
            com_err(prog, ret, _("while converting etype"));
            exit(1);
        }
    } else {
        etype = 0;
    }

    if (ccachestr)
        ret = krb5_cc_resolve(context, ccachestr, &ccache);
    else
        ret = krb5_cc_default(context, &ccache);
    if (ret) {
        com_err(prog, ret, _("while opening ccache"));
        exit(1);
    }

    if (keytab_name != NULL) {
        ret = krb5_kt_resolve(context, keytab_name, &keytab);
        if (ret) {
            com_err(prog, ret, _("resolving keytab %s"), keytab_name);
            exit(1);
        }
    }

    if (for_user) {
        ret = krb5_parse_name_flags(context, for_user,
                                    KRB5_PRINCIPAL_PARSE_ENTERPRISE,
                                    &for_user_princ);
        if (ret) {
            com_err(prog, ret, _("while parsing principal name %s"), for_user);
            exit(1);
        }
    }

    if (u2u_ccname != NULL) {
        ret = get_u2u_ticket(u2u_ccname, &u2u_ticket);
        if (ret) {
            com_err(prog, ret, _("while getting user-to-user ticket from %s"),
                    u2u_ccname);
            exit(1);
        }
        options |= KRB5_GC_USER_USER;
    }

    ret = krb5_cc_get_principal(context, ccache, &me);
    if (ret) {
        com_err(prog, ret, _("while getting client principal name"));
        exit(1);
    }

    errors = 0;
    for (i = 0; i < count; i++) {
        if (kvno(names[i], ccache, me, etype, keytab, sname, options, unknown,
                 for_user_princ, proxy, u2u_ticket) != 0)
            errors++;
    }

    if (keytab != NULL)
        krb5_kt_close(context, keytab);
    krb5_free_principal(context, me);
    krb5_free_principal(context, for_user_princ);
    krb5_cc_close(context, ccache);
    krb5_free_data(context, u2u_ticket);
    krb5_free_context(context);

    if (errors)
        exit(1);

    exit(0);
}
