/***

ipfilter.c - user interface and filter function for all IP packets

Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 2001, 2002

This software is open-source; you may redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <curses.h>
#include <panel.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <input.h>
#include <menurt.h>
#include <msgboxes.h>
#include <labels.h>
#include "addproto.h"
#include "dirs.h"
#include "deskman.h"
#include "attrs.h"
#include "fltdefs.h"
#include "fltmgr.h"
#include "ipfilter.h"
#include "fltedit.h"
#include "getpath.h"
#include "parseproto.h"
#include "cidr.h"

extern int daemonized;

void gethostparams(struct hostparams *data, char *init_saddr,
                   char *init_smask, char *init_sport1, char *init_sport2,
                   char *init_daddr, char *init_dmask, char *init_dport1,
                   char *init_dport2, char *initinex, char *initmatchop,
                   int *aborted)
{
    WINDOW *dlgwin;
    PANEL *dlgpanel;

    struct FIELDLIST fields;
    struct FIELD *fieldptr;

    int rangeproto1, rangeproto2;
    int parse_result;
    char *bptr, *cptr;
    int i, doagain;
    char msgstr[60];
    char actual_address[30];
    unsigned int maskbits;

    const char *init_yesno = "Y";
    const char *WILDCARD = "0.0.0.0";

    dlgwin = newwin(22, 80, (LINES - 22) / 2, (COLS - 80) / 2);
    dlgpanel = new_panel(dlgwin);

    wattrset(dlgwin, DLGBOXATTR);
    tx_colorwin(dlgwin);
    tx_box(dlgwin, ACS_VLINE, ACS_HLINE);

    mvwprintw(dlgwin, 0, 22, " Source ");
    mvwprintw(dlgwin, 0, 52, " Destination ");

    wmove(dlgwin, 20, 2);
    tabkeyhelp(dlgwin);
    stdkeyhelp(dlgwin);
    wattrset(dlgwin, DLGTEXTATTR);
    mvwprintw(dlgwin, 2, 2, "IP address");
    mvwprintw(dlgwin, 4, 2, "Wildcard mask");
    mvwprintw(dlgwin, 6, 2, "Port");
    mvwprintw(dlgwin, 9, 2, "Protocols to match");
    mvwprintw(dlgwin, 10, 2, "(Enter Y beside each");
    mvwprintw(dlgwin, 11, 2, "protocol to match.)");
    mvwprintw(dlgwin, 18, 2, "Include/Exclude (I/E)");

    tx_initfields(&fields, 19, 55, (LINES - 22) / 2 + 1,
                  (COLS - 80) / 2 + 23, DLGTEXTATTR, FIELDATTR);

    mvwprintw(fields.fieldwin, 5, 6, "to");
    mvwprintw(fields.fieldwin, 5, 36, "to");
    mvwprintw(fields.fieldwin, 6, 0,
              "Port fields apply only to TCP and UDP packets");
    mvwprintw(fields.fieldwin, 8, 3, "All IP");
    mvwprintw(fields.fieldwin, 8, 16, "TCP");
    mvwprintw(fields.fieldwin, 8, 26, "UDP");
    mvwprintw(fields.fieldwin, 8, 35, "ICMP");
    mvwprintw(fields.fieldwin, 8, 45, "IGMP");
    mvwprintw(fields.fieldwin, 10, 5, "OSPF");
    mvwprintw(fields.fieldwin, 10, 16, "IGP");
    mvwprintw(fields.fieldwin, 10, 25, "IGRP");
    mvwprintw(fields.fieldwin, 10, 36, "GRE");
    mvwprintw(fields.fieldwin, 10, 45, "L2TP");
    mvwprintw(fields.fieldwin, 12, 1, "IPSec AH");
    mvwprintw(fields.fieldwin, 12, 13, "IPSec ESP");
    mvwprintw(fields.fieldwin, 14, 1,
              "Additional protocols or ranges (e.g. 8, 18-20, 69, 90)");
    mvwprintw(fields.fieldwin, 17, 11, "Match opposite (Y/N)");

    tx_addfield(&fields, 25, 1, 0, init_saddr);
    tx_addfield(&fields, 25, 3, 0, init_smask);
    tx_addfield(&fields, 5, 5, 0, init_sport1);
    tx_addfield(&fields, 5, 5, 9, init_sport2);
    tx_addfield(&fields, 25, 1, 30, init_daddr);
    tx_addfield(&fields, 25, 3, 30, init_dmask);
    tx_addfield(&fields, 5, 5, 30, init_dport1);
    tx_addfield(&fields, 5, 5, 39, init_dport2);

    if (data->filters[F_ALL_IP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 8, 10, init_yesno);

    if (data->filters[F_TCP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 8, 20, init_yesno);

    if (data->filters[F_UDP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 8, 30, init_yesno);

    if (data->filters[F_ICMP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 8, 40, init_yesno);

    if (data->filters[F_IGMP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 8, 50, init_yesno);

    if (data->filters[F_OSPF])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 10, 10, init_yesno);

    if (data->filters[F_IGP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 10, 20, init_yesno);

    if (data->filters[F_IGRP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 10, 30, init_yesno);

    if (data->filters[F_GRE])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 10, 40, init_yesno);

    if (data->filters[F_L2TP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 10, 50, init_yesno);

    if (data->filters[F_IPSEC_AH])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 12, 10, init_yesno);

    if (data->filters[F_IPSEC_ESP])
        init_yesno = "Y";
    else
        init_yesno = "";
    tx_addfield(&fields, 1, 12, 23, init_yesno);

    cptr = tx_ltrim(data->protolist);
    tx_addfield(&fields, 54, 15, 1, cptr);
    tx_addfield(&fields, 1, 17, 1, initinex);
    tx_addfield(&fields, 1, 17, 32, initmatchop);

    do {
        tx_fillfields(&fields, aborted);        /*get input */
        if (!(*aborted)) {
            fieldptr = fields.list;

            /*
             * Adjust upper loop bound depending on the number of fields
             * before the "Additional IP protocols" field.
             */
            for (i = 2; i <= 21; i++)
                fieldptr = fieldptr->nextfield;

            if (!validate_ranges(fieldptr->buf, &parse_result, &bptr)) {
                snprintf(msgstr, 60,
                         "Invalid protocol input at or near token \"%s\"",
                         bptr);
                tx_errbox(msgstr, ANYKEY_MSG, &i);
                doagain = 1;
            } else
                doagain = 0;
        } else {
            doagain = 0;
        }
    } while (doagain);

    /*
     * Store entered filter data into data structures
     */
    if (!(*aborted)) {
        fieldptr = fields.list;
        maskbits = 0;

        /*
         * Process Source Address field
         */
        if (fieldptr->buf[0] == '\0')
            strcpy(data->s_fqdn, WILDCARD);
        else
            strcpy(data->s_fqdn, fieldptr->buf);

        if (strchr(data->s_fqdn, '/') != NULL) {
            cidr_split_address(data->s_fqdn, actual_address, &maskbits);
            strcpy(data->s_fqdn, actual_address);
        }

        /*
         * Process Source Mask field
         */
        fieldptr = fieldptr->nextfield;
        if (fieldptr->buf[0] == '\0') {
            if (maskbits > 32) {
                strcpy(data->s_mask, WILDCARD);
            } else {
                strncpy(data->s_mask, cidr_get_quad_mask(maskbits), 20);
            }
        } else
            strcpy(data->s_mask, fieldptr->buf);

        /*
         * Process Source Port fields
         */
        fieldptr = fieldptr->nextfield;
        if (fieldptr->buf[0] == '\0')
            data->sport1 = 0;
        else
            data->sport1 = atoi(fieldptr->buf);

        fieldptr = fieldptr->nextfield;
        if (fieldptr->buf[0] == '\0')
            data->sport2 = 0;
        else
            data->sport2 = atoi(fieldptr->buf);

        /*
         * Process Destination Address field
         */
        fieldptr = fieldptr->nextfield;
        if (fieldptr->buf[0] == '\0')
            strcpy(data->d_fqdn, WILDCARD);
        else
            strcpy(data->d_fqdn, fieldptr->buf);

        maskbits = 0;
        if (strchr(data->d_fqdn, '/') != NULL) {
            cidr_split_address(data->d_fqdn, actual_address, &maskbits);
            strcpy(data->d_fqdn, actual_address);
        }

        /*
         * Process Destination mask field
         */
        fieldptr = fieldptr->nextfield;
        if (fieldptr->buf[0] == '\0') {
            if (maskbits > 32) {
                strcpy(data->d_mask, WILDCARD);
            } else {
                strncpy(data->d_mask, cidr_get_quad_mask(maskbits), 20);
            }
        } else
            strcpy(data->d_mask, fieldptr->buf);

        /*
         * Process Dedination Port fields
         */
        fieldptr = fieldptr->nextfield;
        if (fieldptr->buf[0] == '\0')
            data->dport1 = 0;
        else
            data->dport1 = atoi(fieldptr->buf);

        fieldptr = fieldptr->nextfield;
        if (fieldptr->buf[0] == '\0')
            data->dport2 = 0;
        else
            data->dport2 = atoi(fieldptr->buf);

        /*
         * Process IP protocol filter fields
         */
        fieldptr = fieldptr->nextfield;
        memset(&(data->filters), 0, sizeof(data->filters));

        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_ALL_IP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_TCP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_UDP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_ICMP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_IGMP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_OSPF] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_IGP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_IGRP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_GRE] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_L2TP] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_IPSEC_AH] = 1;
        fieldptr = fieldptr->nextfield;
        if (toupper(fieldptr->buf[0]) == 'Y')
            data->filters[F_IPSEC_ESP] = 1;
        fieldptr = fieldptr->nextfield;

        /*
         * Parse protocol string
         */
        cptr = fieldptr->buf;
        strncpy(data->protolist, cptr, 60);

        do {
            get_next_protorange(fieldptr->buf, &cptr, &rangeproto1,
                                &rangeproto2, &parse_result, &bptr);
            if (parse_result == RANGE_OK) {
                if (rangeproto2 != 0) {
                    for (i = rangeproto1; i <= rangeproto2; i++) {
                        data->filters[i] = 1;
                    }
                } else {
                    data->filters[rangeproto1] = 1;
                }
            }
        } while (parse_result == RANGE_OK);

        data->reverse = toupper(fieldptr->nextfield->buf[0]);
        if (data->reverse != 'E')
            data->reverse = 'I';

        data->match_opposite =
            toupper(fieldptr->nextfield->nextfield->buf[0]);
        if (data->match_opposite != 'Y')
            data->match_opposite = 'N';
    }

    tx_destroyfields(&fields);
    del_panel(dlgpanel);
    delwin(dlgwin);
    update_panels();
    doupdate();
}

void ipfilterselect(struct filterlist *fl,
                    char *filename, int *fltcode, int *aborted)
{
    struct MENU menu;
    int row = 1;
    struct filterfileent fflist;
    makestdfiltermenu(&menu);
    do {
        tx_showmenu(&menu);
        tx_operatemenu(&menu, &row, aborted);
        switch (row) {
        case 1:
            definefilter(aborted);
            break;
        case 2:
            selectfilter(&fflist, aborted);
            if (!(*aborted)) {
                memset(filename, 0, FLT_FILENAME_MAX);
                strncpy(filename, get_path(T_WORKDIR, fflist.filename),
                        FLT_FILENAME_MAX - 1);
                if (!loadfilter(filename, fl, FLT_RESOLVE))
                    *fltcode = 1;
                else
                    *fltcode = 0;
            }
            break;
        case 3:
            destroyfilter(fl);
            *fltcode = 0;
            tx_infobox("IP filter deactivated", ANYKEY_MSG);
            break;
        case 4:
            editfilter(aborted);
            break;
        case 5:
            delfilter(aborted);
            if (!(*aborted))
                tx_infobox("IP filter deleted", ANYKEY_MSG);
        }
    } while (row != 7);
    tx_destroymenu(&menu);
    update_panels();
    doupdate();
}

/*
 * Display/logging filter for other (non-TCP, non-UDP) IP protocols.
 */
int ipfilter(unsigned long saddr, unsigned long daddr,
             unsigned int sport, unsigned int dport,
             unsigned int protocol, int match_opp_mode,
             struct filterlist *fl)
{
    struct filterent *fe = fl->head;
    int result = 0;
    int fltexpr1;
    int fltexpr2;


    while (fe != NULL) {
        if (protocol == IPPROTO_TCP || protocol == IPPROTO_UDP) {
            fltexpr1 = ((saddr & fe->smask) == (fe->saddr & fe->smask)
                        && (daddr & fe->dmask) == (fe->daddr & fe->dmask))
                &&
                (((fe->hp.sport2 == 0
                   && (fe->hp.sport1 == sport || fe->hp.sport1 == 0))
                  || (fe->hp.sport2 != 0
                      && (sport >= fe->hp.sport1
                          && sport <= fe->hp.sport2)))
                 &&
                 ((fe->hp.dport2 == 0
                   && (fe->hp.dport1 == dport || fe->hp.dport1 == 0))
                  || (fe->hp.dport2 != 0
                      && (dport >= fe->hp.dport1
                          && dport <= fe->hp.dport2))));

            if ((protocol == IPPROTO_TCP
                 && match_opp_mode == MATCH_OPPOSITE_ALWAYS)
                || (fe->hp.match_opposite == 'Y'))
                fltexpr2 = ((saddr & fe->dmask) == (fe->daddr & fe->dmask)
                            && (daddr & fe->smask) ==
                            (fe->saddr & fe->smask))
                    &&
                    (((fe->hp.dport2 == 0
                       && (sport == fe->hp.dport1 || fe->hp.dport1 == 0))
                      || (fe->hp.dport2 != 0
                          && (sport >= fe->hp.dport1
                              && sport <= fe->hp.dport2)))
                     &&
                     ((fe->hp.sport2 == 0
                       && (dport == fe->hp.sport1 || fe->hp.sport1 == 0))
                      || (fe->hp.dport2 != 0
                          && (dport >= fe->hp.sport1
                              && dport <= fe->hp.sport2))));
            else
                fltexpr2 = 0;
        } else {
            fltexpr1 = ((saddr & fe->smask) == (fe->saddr & fe->smask))
                && ((daddr & fe->dmask) == (fe->daddr & fe->dmask));

            if (fe->hp.match_opposite == 'Y') {
                fltexpr2 = ((daddr & fe->smask) == (fe->saddr & fe->smask))
                    && ((saddr & fe->dmask) == (fe->daddr & fe->dmask));
            } else
                fltexpr2 = 0;
        }

        if (fltexpr1 || fltexpr2) {
            result = fe->hp.filters[protocol] || fe->hp.filters[F_ALL_IP];

            if (result) {
                if (toupper(fe->hp.reverse) == 'E') {
                    return 0;
                }

                return 1;
            }
        }
        fe = fe->next_entry;
    }

    return 0;
}
