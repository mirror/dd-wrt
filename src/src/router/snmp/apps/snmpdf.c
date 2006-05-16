/*
 * snmpget.c - send snmp GET requests to a network entity.
 *
 */
/***********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
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

int             failures = 0;

void
usage(void)
{
    fprintf(stderr, "Usage: snmpdf [-Cu] ");
    snmp_parse_args_usage(stderr);
    fprintf(stderr, "\n\n");
    snmp_parse_args_descriptions(stderr);
    fprintf(stderr, "\nsnmpdf options:\n");
    fprintf(stderr,
            "\t-Cu\tUse UCD-SNMP dskTable to do the calculations.\n");
    fprintf(stderr,
            "\t\t[Normally the HOST-RESOURCES-MIB is consulted first.]\n");
}

int             ucd_mib = 0;

static void
optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'u':
                ucd_mib = 1;
                break;
            default:
                fprintf(stderr,
                        "Unknown flag passed to -C: %c\n", optarg[-1]);
                exit(1);
            }
        }
    }
}

struct hrStorageTable {
    u_long          hrStorageIndex;
    oid            *hrStorageType;
    char           *hrStorageDescr;
    u_long          hrStorageAllocationUnits;
    u_long          hrStorageSize;
    u_long          hrStorageUsed;
};

int
add(netsnmp_pdu *pdu, const char *mibnodename,
    oid * index, size_t indexlen)
{
    oid             base[MAX_OID_LEN];
    size_t          base_length = MAX_OID_LEN;

    memset(base, 0, MAX_OID_LEN * sizeof(oid));

    if (!snmp_parse_oid(mibnodename, base, &base_length)) {
        snmp_perror(mibnodename);
        fprintf(stderr, "couldn't find mib node %s, giving up\n",
                mibnodename);
        exit(1);
    }

    if (index && indexlen) {
        memcpy(&(base[base_length]), index, indexlen * sizeof(oid));
        base_length += indexlen;
    }
    DEBUGMSGTL(("add", "created: "));
    DEBUGMSGOID(("add", base, base_length));
    DEBUGMSG(("add", "\n"));
    snmp_add_null_var(pdu, base, base_length);

    return base_length;
}

netsnmp_variable_list *
collect(netsnmp_session * ss, netsnmp_pdu *pdu,
        oid * base, size_t base_length)
{
    netsnmp_pdu    *response;
    int             running = 1;
    netsnmp_variable_list *saved = NULL, **vlpp = &saved;
    int             status;

    while (running) {
        /*
         * gotta catch em all, gotta catch em all! 
         */
        status = snmp_synch_response(ss, pdu, &response);
        if (status != STAT_SUCCESS || !response) {
            snmp_sess_perror("snmpdf", ss);
            exit(1);
        }
        if (response && snmp_oid_compare(response->variables->name,
                                         SNMP_MIN(base_length,
                                                  response->variables->
                                                  name_length), base,
                                         base_length) != 0)
            running = 0;
        else {
            /*
             * get response 
             */
            *vlpp = response->variables;
            (*vlpp)->next_variable = NULL;      /* shouldn't be any, but just in case */

            /*
             * create the next request 
             */
            pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
            snmp_add_null_var(pdu, (*vlpp)->name, (*vlpp)->name_length);

            /*
             * finish loop setup 
             */
            vlpp = &((*vlpp)->next_variable);
            response->variables = NULL; /* ahh, forget about it */
        }
        snmp_free_pdu(response);
    }
    return saved;
}



int
main(int argc, char *argv[])
{
    netsnmp_session session, *ss;
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    int             arg;
    oid             base[MAX_OID_LEN];
    size_t          base_length;
    int             status;
    netsnmp_variable_list *saved = NULL, *vlp = saved, *vlp2;
    int             count = 0;

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
     * Open an SNMP session.
     */
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
        snmp_sess_perror("snmpget", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    printf("%-18s %15s %15s %15s %5s\n", "Description", "size (kB)",
           "Used", "Available", "Used%");
    if (ucd_mib == 0) {
        /*
         * * Begin by finding all the storage pieces that are of
         * * type hrStorageFixedDisk, which is a standard disk.
         */
        pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
        base_length =
            add(pdu, "HOST-RESOURCES-MIB:hrStorageIndex", NULL, 0);
        memcpy(base, pdu->variables->name, base_length * sizeof(oid));

        vlp = collect(ss, pdu, base, base_length);

        while (vlp) {
            size_t          units;
            unsigned long   hssize, hsused;
            char            descr[SPRINT_MAX_LEN];

            pdu = snmp_pdu_create(SNMP_MSG_GET);

            add(pdu, "HOST-RESOURCES-MIB:hrStorageDescr",
                &(vlp->name[base_length]), vlp->name_length - base_length);
            add(pdu, "HOST-RESOURCES-MIB:hrStorageAllocationUnits",
                &(vlp->name[base_length]), vlp->name_length - base_length);
            add(pdu, "HOST-RESOURCES-MIB:hrStorageSize",
                &(vlp->name[base_length]), vlp->name_length - base_length);
            add(pdu, "HOST-RESOURCES-MIB:hrStorageUsed",
                &(vlp->name[base_length]), vlp->name_length - base_length);

            status = snmp_synch_response(ss, pdu, &response);
            if (status != STAT_SUCCESS || !response) {
                snmp_sess_perror("snmpdf", ss);
                exit(1);
            }

            vlp2 = response->variables;
            memcpy(descr, vlp2->val.string, vlp2->val_len);
            descr[vlp2->val_len] = '\0';

            vlp2 = vlp2->next_variable;
            units = vlp2->val.integer ? *(vlp2->val.integer) : 0;

            vlp2 = vlp2->next_variable;
            hssize = vlp2->val.integer ? *(vlp2->val.integer) : 0;

            vlp2 = vlp2->next_variable;
            hsused = vlp2->val.integer ? *(vlp2->val.integer) : 0;

            printf("%-18s %15lu %15lu %15lu %4lu%%\n", descr,
                   units ? hssize * (units / 1024) : hssize,
                   units ? hsused * (units / 1024) : hsused,
                   units ? (hssize - hsused) * (units / 1024) : hssize -
                   hsused, hssize ? 100 * hsused / hssize : hsused);

            vlp = vlp->next_variable;
            snmp_free_pdu(response);
            count++;
        }
    }

    if (count == 0) {
        size_t          units = 0;
        /*
         * the host resources mib must not be supported.  Lets try the
         * UCD-SNMP-MIB and its dskTable 
         */

        pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
        base_length = add(pdu, "UCD-SNMP-MIB:dskIndex", NULL, 0);
        memcpy(base, pdu->variables->name, base_length * sizeof(oid));

        vlp = collect(ss, pdu, base, base_length);

        while (vlp) {
            unsigned long   hssize, hsused;
            char            descr[SPRINT_MAX_LEN];

            pdu = snmp_pdu_create(SNMP_MSG_GET);

            add(pdu, "UCD-SNMP-MIB:dskPath",
                &(vlp->name[base_length]), vlp->name_length - base_length);
            add(pdu, "UCD-SNMP-MIB:dskTotal",
                &(vlp->name[base_length]), vlp->name_length - base_length);
            add(pdu, "UCD-SNMP-MIB:dskUsed",
                &(vlp->name[base_length]), vlp->name_length - base_length);

            status = snmp_synch_response(ss, pdu, &response);
            if (status != STAT_SUCCESS || !response) {
                snmp_sess_perror("snmpdf", ss);
                exit(1);
            }

            vlp2 = response->variables;
            memcpy(descr, vlp2->val.string, vlp2->val_len);
            descr[vlp2->val_len] = '\0';

            vlp2 = vlp2->next_variable;
            hssize = *(vlp2->val.integer);

            vlp2 = vlp2->next_variable;
            hsused = *(vlp2->val.integer);

            printf("%-18s %15lu %15lu %15lu %4lu%%\n", descr,
                   units ? hssize * (units / 1024) : hssize,
                   units ? hsused * (units / 1024) : hsused,
                   units ? (hssize - hsused) * (units / 1024) : hssize -
                   hsused, hssize ? 100 * hsused / hssize : hsused);

            vlp = vlp->next_variable;
            snmp_free_pdu(response);
            count++;
        }
    }

    if (count == 0) {
        fprintf(stderr, "Failed to locate any partions.\n");
        exit(1);
    }

    snmp_close(ss);
    SOCK_CLEANUP;
    return 0;

}                               /* end main() */
