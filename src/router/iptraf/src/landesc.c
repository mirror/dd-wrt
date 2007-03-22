
/***

landesc.c	- LAN host description management module
		  Currently includes support for Ethernet, PLIP,
		  and FDDI

Copyright (c) Gerard Paul Java 1998

This software is open source; you can redistribute it and/or modify
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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <ctype.h>
#include <winops.h>
#include <menurt.h>
#include <input.h>
#include <labels.h>
#include <msgboxes.h>
#include <listbox.h>
#include "deskman.h"
#include "attrs.h"
#include "dirs.h"
#include "landesc.h"
#include "links.h"

void etherr(void)
{
    int resp;

    tx_errbox("Unable to open host description file", ANYKEY_MSG, &resp);
}

void add_desclist_node(struct desclist *list, struct desclistent *ptmp)
{
    if (list->head == NULL) {
        list->head = ptmp;
        ptmp->prev_entry = NULL;
    } else {
        list->tail->next_entry = ptmp;
        ptmp->prev_entry = list->tail;
    }

    ptmp->next_entry = NULL;
    list->tail = ptmp;
}

/*
 * Loads descriptions from the IPTraf LAN host description files.
 * Now also loads /etc/ethers as well.
 *
 * In case of a duplicate in the IPTraf definition files and /etc/ethers,
 * the IPTraf definition files take precedence.
 */

void loaddesclist(struct desclist *list, unsigned int linktype,
                  int withethers)
{
    struct desclistent *ptmp = NULL;
    FILE *fd = NULL;
    char descline[140];
    char *desctoken;
    char etherline[140];
    int i, j;                   /* counters used when parsing /etc/ethers */

    bzero(list, sizeof(struct desclist));

    if (linktype == LINK_ETHERNET)
        fd = fopen(ETHFILE, "r");
    else if (linktype == LINK_FDDI)
        fd = fopen(FDDIFILE, "r");

    if (fd == NULL) {
        return;
    }
    while (!feof(fd)) {
        ptmp = malloc(sizeof(struct desclistent));
        if (ptmp == NULL) {
            printnomem();
            return;
        }
        bzero(ptmp, sizeof(struct desclistent));
        bzero(descline, 140);
        fgets(descline, 140, fd);

        if (strcmp(descline, "") == 0) {
            free(ptmp);
            continue;
        }
        strncpy(ptmp->rec.address, strtok(descline, ":"), 12);
        desctoken = strtok(NULL, "\n");

        if (desctoken != NULL)
            strncpy(ptmp->rec.desc, desctoken, 64);
        else
            strcpy(ptmp->rec.desc, "");

        add_desclist_node(list, ptmp);
    }

    fclose(fd);

    /* 
     * Loads MAC addresses defined in /etc/ethers.  Contributed by
     * Debian maintainter Frederic Peters <fpeters@debian.org>.  Thanks
     * Frederic!
     *
     * Contributor's note:
     *  loading other ethenet mac addresses from /etc/ethers (used by tcpdump)
     *
     * Author's note:
     *  Moved significantly repeating code to a function.
     */

    if (!withethers)
        return;

    if (linktype != LINK_ETHERNET)
        return;

    fd = fopen("/etc/ethers", "r");

    if (fd == NULL)
        return;

    while (!feof(fd)) {
        ptmp = malloc(sizeof(struct desclistent));
        if (ptmp == NULL) {
            printnomem();
            return;
        }
        bzero(ptmp, sizeof(struct desclistent));
        bzero(descline, 140);
        bzero(etherline, 140);
        fgets(etherline, 140, fd);

        /* 
         * Convert /etc/ethers line to a descline
         */
        if (etherline[0] == '#' || etherline[0] == '\n'
            || etherline[0] == 0) {
            free(ptmp);
            continue;
        }

        if (strchr(etherline, '\n'))
            strchr(etherline, '\n')[0] = 0;

        j = 0;
        for (i = 0; i < 20 && !isspace(etherline[i]); i++) {
            if (etherline[i] == ':')
                continue;
            descline[j++] = tolower(etherline[i]);
        }
        descline[j] = ':';

        /* 
         * Skip over whitespace between MAC address and IP addr/host name
         */

        while (isspace(etherline[i++]));

        strncat(descline, etherline + i - 1, 130);

        if (strcmp(descline, "") == 0) {
            free(ptmp);
            continue;
        }

        strncpy(ptmp->rec.address, strtok(descline, ":"), 12);
        desctoken = strtok(NULL, "\n");

        if (desctoken != NULL)
            strncpy(ptmp->rec.desc, desctoken, 64);
        else
            strcpy(ptmp->rec.desc, "");

        add_desclist_node(list, ptmp);
    }

    fclose(fd);
}

void savedesclist(struct desclist *list, unsigned int linktype)
{
    FILE *fd = NULL;

    struct desclistent *ptmp = list->head;

    if (linktype == LINK_ETHERNET)
        fd = fopen(ETHFILE, "w");
    else if (linktype == LINK_FDDI)
        fd = fopen(FDDIFILE, "w");

    if (fd < 0) {
        etherr();
        return;
    }
    while (ptmp != NULL) {
        fprintf(fd, "%s:%s\n", ptmp->rec.address, ptmp->rec.desc);
        ptmp = ptmp->next_entry;
    }

    fclose(fd);
}

void displayethdescs(struct desclist *list, WINDOW * win)
{
    struct desclistent *ptmp = list->head;
    short i = 0;

    do {
        wmove(win, i, 2);
        wprintw(win, "%s    %s", ptmp->rec.address, ptmp->rec.desc);
        i++;
        ptmp = ptmp->next_entry;
    } while ((i < 18) && (ptmp != NULL));

    update_panels();
    doupdate();
}

void destroydesclist(struct desclist *list)
{
    struct desclistent *ptmp = list->head;
    struct desclistent *ctmp = NULL;

    if (list->head != NULL)
        ctmp = ptmp->next_entry;

    while (ptmp != NULL) {
        free(ptmp);
        ptmp = ctmp;

        if (ctmp != NULL)
            ctmp = ctmp->next_entry;
    }
}

void operate_descselect(struct desclist *list, struct desclistent **node,
                        WINDOW * win, int *aborted)
{
    int ch = 0;
    int i = 0;
    char sp_buf[10];

    int exitloop = 0;

    *node = list->head;

    sprintf(sp_buf, "%%%dc", COLS - 2);

    do {
        wattrset(win, PTRATTR);
        wmove(win, i, 1);
        waddch(win, ACS_RARROW);
        ch = wgetch(win);
        wmove(win, i, 1);
        waddch(win, ' ');
        wattrset(win, STDATTR);

        switch (ch) {
        case KEY_DOWN:
            if ((*node)->next_entry != NULL) {
                *node = (*node)->next_entry;

                if (i < 17)
                    i++;
                else {
                    wscrl(win, 1);
                    scrollok(win, 0);
                    wmove(win, 17, 0);
                    wprintw(win, sp_buf, ' ');
                    scrollok(win, 1);
                    wmove(win, 17, 2);
                    wprintw(win, "%s    %s", (*node)->rec.address,
                            (*node)->rec.desc);
                }
            }
            break;
        case KEY_UP:
            if ((*node)->prev_entry != NULL) {
                *node = (*node)->prev_entry;

                if (i > 0)
                    i--;
                else {
                    wscrl(win, -1);
                    wmove(win, 0, 0);
                    wprintw(win, sp_buf, ' ');
                    wmove(win, 0, 2);
                    wprintw(win, "%s    %s", (*node)->rec.address,
                            (*node)->rec.desc);
                }
            }
            break;
        case 13:
            exitloop = 1;
            *aborted = 0;
            break;
        case 27:
        case 24:
        case 'x':
        case 'X':
        case 'q':
        case 'Q':
            exitloop = 1;
            *aborted = 1;
            break;
        }
    } while (!exitloop);
}

void selectdesc(struct desclist *list, struct desclistent **node,
                int *aborted)
{
    int resp;
    struct scroll_list slist;
    char descline[80];

    if (list->head == NULL) {
        tx_errbox("No descriptions", ANYKEY_MSG, &resp);
        return;
    }

    *node = list->head;
    tx_init_listbox(&slist, COLS, 20, 0, (LINES - 20) / 2,
                    STDATTR, BOXATTR, BARSTDATTR, HIGHATTR);

    tx_set_listbox_title(&slist, "Address", 1);
    tx_set_listbox_title(&slist, "Description", 19);

    while (*node != NULL) {
        snprintf(descline, 80, "%-18s%s", (*node)->rec.address,
                 (*node)->rec.desc);
        tx_add_list_entry(&slist, (char *) (*node), descline);
        (*node) = (*node)->next_entry;
    }

    tx_show_listbox(&slist);
    tx_operate_listbox(&slist, &resp, aborted);

    if (!(*aborted))
        *node = (struct desclistent *) slist.textptr->nodeptr;

    tx_close_listbox(&slist);
    tx_destroy_list(&slist);

    update_panels();
    doupdate();
}

void descdlg(struct descrec *rec, char *initaddr, char *initdesc,
             int *aborted)
{
    WINDOW *win;
    PANEL *panel;
    struct FIELDLIST fieldlist;

    win = newwin(8, 70, 8, (COLS - 70) / 2);
    panel = new_panel(win);

    wattrset(win, DLGBOXATTR);
    tx_colorwin(win);
    tx_box(win, ACS_VLINE, ACS_HLINE);
    wmove(win, 6, 2 * COLS / 80);
    tabkeyhelp(win);
    wmove(win, 6, 20 * COLS / 80);
    stdkeyhelp(win);

    wattrset(win, DLGTEXTATTR);
    wmove(win, 2, 2 * COLS / 80);
    wprintw(win, "MAC Address:");
    wmove(win, 4, 2 * COLS / 80);
    wprintw(win, "Description:");

    tx_initfields(&fieldlist, 3, 52, 10, (COLS - 52) / 2 + 6 * COLS / 80,
                  DLGTEXTATTR, FIELDATTR);
    tx_addfield(&fieldlist, 12, 0, 0, initaddr);
    tx_addfield(&fieldlist, 50, 2, 0, initdesc);

    tx_fillfields(&fieldlist, aborted);

    if (!(*aborted)) {
        strcpy(rec->address, fieldlist.list->buf);
        strcpy(rec->desc, fieldlist.list->nextfield->buf);
    }
    tx_destroyfields(&fieldlist);
    del_panel(panel);
    delwin(win);
}

void addethdesc(struct desclist *list)
{
    struct descrec rec;
    int aborted;
    struct desclistent *ptmp;

    descdlg(&rec, "", "", &aborted);

    if (!aborted) {
        ptmp = malloc(sizeof(struct desclistent));
        if (list->head == NULL) {
            list->head = ptmp;
            ptmp->prev_entry = NULL;
        } else {
            ptmp->prev_entry = list->tail;
            list->tail->next_entry = ptmp;
        }

        list->tail = ptmp;
        ptmp->next_entry = NULL;
        memcpy(&(ptmp->rec), &rec, sizeof(struct descrec));
    }
    update_panels();
    doupdate();
}

void editethdesc(struct desclist *list)
{
    struct desclistent *ptmp;
    int aborted;

    selectdesc(list, &ptmp, &aborted);

    if (!aborted)
        descdlg(&(ptmp->rec), ptmp->rec.address, ptmp->rec.desc, &aborted);
}

void delethdesc(struct desclist *list)
{
    struct desclistent *ptmp;
    int aborted;

    selectdesc(list, &ptmp, &aborted);

    if (!aborted) {
        if (ptmp->prev_entry != NULL)
            ptmp->prev_entry->next_entry = ptmp->next_entry;
        else
            list->head = ptmp->next_entry;

        if (ptmp->next_entry != NULL)
            ptmp->next_entry->prev_entry = ptmp->prev_entry;
        else
            list->tail = ptmp->prev_entry;

        free(ptmp);
    }
}


void ethdescmgr(unsigned int linktype)
{
    struct MENU menu;
    int row = 1;
    int aborted;
    struct desclist list;

    loaddesclist(&list, linktype, WITHOUTETCETHERS);

    tx_initmenu(&menu, 7, 31, (LINES - 6) / 2, (COLS - 31) / 2,
                BOXATTR, STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR,
                DESCATTR);
    tx_additem(&menu, " ^A^dd description...",
               "Adds a description for a MAC address");
    tx_additem(&menu, " ^E^dit description...",
               "Modifies an existing MAC address description");
    tx_additem(&menu, " ^D^elete description...",
               "Deletes an existing MAC address description");
    tx_additem(&menu, NULL, NULL);
    tx_additem(&menu, " E^x^it menu", "Returns to the main menu");

    do {
        tx_showmenu(&menu);
        tx_operatemenu(&menu, &row, &aborted);

        switch (row) {
        case 1:
            addethdesc(&list);
            break;
        case 2:
            editethdesc(&list);
            break;
        case 3:
            delethdesc(&list);
            break;
        }
    } while (row != 5);

    tx_destroymenu(&menu);
    update_panels();
    doupdate();
    savedesclist(&list, linktype);
}
