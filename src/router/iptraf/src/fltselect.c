
/***
 
fltselect.c - a menu-based module that allows selection of
	other protocols to display
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997, 1998

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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <winops.h>
#include <menurt.h>
#include <msgboxes.h>
#include "addproto.h"
#include "dirs.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "fltedit.h"
#include "fltmgr.h"
#include "deskman.h"
#include "attrs.h"
#include "instances.h"

void makemainfiltermenu(struct MENU *menu)
{
    tx_initmenu(menu, 8, 18, (LINES - 8) / 2, (COLS - 31) / 2,
                BOXATTR, STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR,
                DESCATTR);
    tx_additem(menu, " ^I^P...", "Manages IP packet filters");
    tx_additem(menu, " ^A^RP",
               "Toggles Address Resolution Protocol filter");
    tx_additem(menu, " ^R^ARP", "Toggles Reverse ARP filter");
    tx_additem(menu, " ^N^on-IP",
               "Toggles filter for all other non-IP packets");
    tx_additem(menu, NULL, NULL);
    tx_additem(menu, " E^x^it menu",
               "Returns to the filter management menu");
}

void setfilters(struct filterstate *filter, unsigned int row)
{
    int aborted;

    switch (row) {
    case 1:
        ipfilterselect(&(filter->fl), filter->filename,
                       &(filter->filtercode), &aborted);
        break;
    case 2:
        filter->arp = ~(filter->arp);
        break;
    case 3:
        filter->rarp = ~(filter->rarp);
        break;
    case 4:
        filter->nonip = ~(filter->nonip);
        break;
    }
}

void toggleprotodisplay(WINDOW * win, struct filterstate *filter,
                        unsigned int row)
{
    wmove(win, row, 2);
    switch (row) {
    case 1:
        if (filter->filtercode == 0)
            wprintw(win, "No IP filter active");
        else
            wprintw(win, "IP filter active   ");
        break;
    case 2:
        if (filter->arp)
            wprintw(win, "ARP visible    ");
        else
            wprintw(win, "ARP not visible");

        break;
    case 3:
        if (filter->rarp)
            wprintw(win, "RARP visible    ");
        else
            wprintw(win, "RARP not visible");

        break;
    case 4:
        if (filter->nonip)
            wprintw(win, "Non-IP visible    ");
        else
            wprintw(win, "Non-IP not visible");

        break;
    }
}

/*
 * Filter for non-IP packets
 */
int nonipfilter(struct filterstate *filter, unsigned int protocol)
{
    int result = 0;

    switch (protocol) {
    case ETH_P_ARP:
        result = filter->arp;
        break;
    case ETH_P_RARP:
        result = filter->rarp;
        break;
    case 0:
        result = filter->nonip;
        break;
    }

    return result;
}

void config_filters(struct filterstate *filter)
{
    struct MENU menu;
    WINDOW *statwin;
    PANEL *statpanel;
    int row;
    int aborted;

    statwin = newwin(6, 30, (LINES - 8) / 2, (COLS - 15) / 2 + 10);
    statpanel = new_panel(statwin);
    wattrset(statwin, BOXATTR);
    tx_colorwin(statwin);
    tx_box(statwin, ACS_VLINE, ACS_HLINE);
    tx_stdwinset(statwin);
    wmove(statwin, 0, 1);
    wprintw(statwin, " Filter Status ");
    wattrset(statwin, STDATTR);

    for (row = 1; row <= 4; row++)
        toggleprotodisplay(statwin, filter, row);

    makemainfiltermenu(&menu);

    row = 1;
    do {
        tx_showmenu(&menu);
        tx_operatemenu(&menu, &row, &aborted);
        setfilters(filter, row);
        toggleprotodisplay(statwin, filter, row);
    } while (row != 6);

    tx_destroymenu(&menu);
    del_panel(statpanel);
    delwin(statwin);
    update_panels();
    doupdate();
}

void setodefaults(struct filterstate *filter)
{
    memset(filter, 0, sizeof(struct filterstate));
    filter->filtercode = 0;
}

void loadfilters(struct filterstate *filter)
{
    int pfd;
    int br;

    pfd = open(FLTSTATEFILE, O_RDONLY); /* open filter state file */

    if (pfd < 0) {
        setodefaults(filter);
        return;
    }
    br = read(pfd, filter, sizeof(struct filterstate));
    if (br < 0)
        setodefaults(filter);

    close(pfd);

    /*
     * Reload IP filter if one was previously applied
     */

    if (filter->filtercode != 0)
        loadfilter(filter->filename, &(filter->fl), FLT_RESOLVE);
}

void savefilters(struct filterstate *filter)
{
    int pfd;
    int bw;
    int resp;

    if (!facility_active(FLTIDFILE, ""))
        mark_facility(FLTIDFILE, "Filter configuration change", "");
    else {
        tx_errbox("Filter state file currently in use; try again later",
                  ANYKEY_MSG, &resp);
        return;
    }

    pfd =
        open(FLTSTATEFILE, O_CREAT | O_TRUNC | O_WRONLY,
             S_IRUSR | S_IWUSR);
    bw = write(pfd, filter, sizeof(struct filterstate));
    if (bw < 1)
        tx_errbox("Unable to write filter state information", ANYKEY_MSG,
                  &resp);

    close(pfd);

    unmark_facility(FLTIDFILE, "");
}
