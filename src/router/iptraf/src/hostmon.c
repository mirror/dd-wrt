
/***

hostmon.c - Host traffic monitor
Discovers LAN hosts and displays packet statistics for them
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
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <asm/types.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_fddi.h>
#include <linux/if_tr.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <winops.h>
#include <labels.h>
#include "dirs.h"
#include "deskman.h"
#include "links.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "isdntab.h"            /* needed by packet.h */
#include "packet.h"
#include "ifaces.h"
#include "hostmon.h"
#include "attrs.h"
#include "log.h"
#include "timer.h"
#include "landesc.h"
#include "options.h"
#include "instances.h"
#include "mode.h"
#include "logvars.h"
#include "promisc.h"
#include "error.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

extern int exitloop;
extern int daemonized;

/* 
 * from log.c, applicable only to this module 
 */

extern void writeethlog(struct ethtabent *list, int units,
                        unsigned long nsecs, FILE * logfile);
extern char *ltrim(char *buf);

/*
 * SIGUSR1 logfile rotation handler
 */

void rotate_lanlog()
{
    rotate_flag = 1;
    strcpy(target_logname, current_logfile);
    signal(SIGUSR1, rotate_lanlog);
}

void ethlook(struct desclist *list, char *address, char *target)
{
    struct desclistent *ptmp = list->head;

    while (ptmp != NULL) {
        if (strcmp(address, ptmp->rec.address) == 0) {
            strcpy(target, ptmp->rec.desc);
            return;
        }
        ptmp = ptmp->next_entry;
    }
}

void initethtab(struct ethtab *table, int unit)
{
    char unitstring[7];

    table->head = table->tail = NULL;
    table->firstvisible = table->lastvisible = NULL;
    table->count = table->entcount = 0;

    table->borderwin = newwin(LINES - 2, COLS, 1, 0);
    table->borderpanel = new_panel(table->borderwin);

    table->tabwin = newwin(LINES - 4, COLS - 2, 2, 1);
    table->tabpanel = new_panel(table->tabwin);

    wattrset(table->borderwin, BOXATTR);
    tx_box(table->borderwin, ACS_VLINE, ACS_HLINE);
    wmove(table->borderwin, 0, 5 * COLS / 80);
    wprintw(table->borderwin, " PktsIn ");
    wmove(table->borderwin, 0, 16 * COLS / 80);
    wprintw(table->borderwin, " IP In ");
    wmove(table->borderwin, 0, 24 * COLS / 80);
    wprintw(table->borderwin, " BytesIn ");
    wmove(table->borderwin, 0, 34 * COLS / 80);
    wprintw(table->borderwin, " InRate ");

    wmove(table->borderwin, 0, 42 * COLS / 80);
    wprintw(table->borderwin, " PktsOut ");
    wmove(table->borderwin, 0, 53 * COLS / 80);
    wprintw(table->borderwin, " IP Out ");
    wmove(table->borderwin, 0, 61 * COLS / 80);
    wprintw(table->borderwin, " BytesOut ");
    wmove(table->borderwin, 0, 70 * COLS / 80);
    wprintw(table->borderwin, " OutRate ");

    wmove(table->borderwin, LINES - 3, 40);

    dispmode(unit, unitstring);

    wprintw(table->borderwin, " InRate and OutRate are in %s/sec ",
            unitstring);

    wattrset(table->tabwin, STDATTR);
    tx_colorwin(table->tabwin);
    tx_stdwinset(table->tabwin);
    wtimeout(table->tabwin, -1);

    update_panels();
    doupdate();
}

struct ethtabent *addethnode(struct ethtab *table, int *nomem)
{
    struct ethtabent *ptemp;

    ptemp = malloc(sizeof(struct ethtabent));

    if (ptemp == NULL) {
        printnomem();
        *nomem = 1;
        return NULL;
    }
    if (table->head == NULL) {
        ptemp->prev_entry = NULL;
        table->head = ptemp;
        table->firstvisible = ptemp;
    } else {
        ptemp->prev_entry = table->tail;
        table->tail->next_entry = ptemp;
    }

    table->tail = ptemp;
    ptemp->next_entry = NULL;

    table->count++;
    ptemp->index = table->count;

    if (table->count <= LINES - 4)
        table->lastvisible = ptemp;

    return ptemp;
}

void convmacaddr(char *addr, char *result)
{
    unsigned int i;
    u_int8_t *ptmp = addr;
    char hexbyte[3];

    strcpy(result, "");
    for (i = 0; i <= 5; i++) {
        sprintf(hexbyte, "%02x", *ptmp);
        strcat(result, hexbyte);
        ptmp++;
    }
}

struct ethtabent *addethentry(struct ethtab *table, unsigned int linktype,
                              char *ifname, char *addr, int *nomem,
                              struct desclist *list)
{
    struct ethtabent *ptemp;

    ptemp = addethnode(table, nomem);

    if (ptemp == NULL)
        return NULL;

    ptemp->type = 0;
    memcpy(&(ptemp->un.desc.eth_addr), addr, ETH_ALEN);
    strcpy(ptemp->un.desc.desc, "");
    convmacaddr(addr, ptemp->un.desc.ascaddr);
    ptemp->un.desc.linktype = linktype;
    ethlook(list, ptemp->un.desc.ascaddr, ptemp->un.desc.desc);
    strcpy(ptemp->un.desc.ifname, ifname);

    if (strcmp(ptemp->un.desc.desc, "") == 0)
        ptemp->un.desc.withdesc = 0;
    else
        ptemp->un.desc.withdesc = 1;

    ptemp->un.desc.printed = 0;

    ptemp = addethnode(table, nomem);

    if (ptemp == NULL)
        return NULL;

    ptemp->type = 1;
    ptemp->un.figs.inpcount = ptemp->un.figs.inpktact = 0;
    ptemp->un.figs.outpcount = ptemp->un.figs.outpktact = 0;
    ptemp->un.figs.inspanbr = ptemp->un.figs.outspanbr = 0;
    ptemp->un.figs.inippcount = ptemp->un.figs.outippcount = 0;
    ptemp->un.figs.inbcount = ptemp->un.figs.outbcount = 0;
    ptemp->un.figs.inrate = ptemp->un.figs.outrate = 0;
    ptemp->un.figs.past5 = 0;

    table->entcount++;

    wmove(table->borderwin, LINES - 3, 1);
    wprintw(table->borderwin, " %u entries ", table->entcount);

    return ptemp;
}

struct ethtabent *in_ethtable(struct ethtab *table, unsigned int linktype,
                              char *addr)
{
    struct ethtabent *ptemp = table->head;

    while (ptemp != NULL) {
        if ((ptemp->type == 0) &&
            (memcmp(addr, ptemp->un.desc.eth_addr, ETH_ALEN) == 0) &&
            (ptemp->un.desc.linktype == linktype))
            return ptemp->next_entry;

        ptemp = ptemp->next_entry;
    }

    return NULL;
}

void updateethent(struct ethtabent *entry, int pktsize,
                  int is_ip, int inout)
{
    if (inout == 0) {
        entry->un.figs.inpcount++;
        entry->un.figs.inbcount += pktsize;
        entry->un.figs.inspanbr += pktsize;
        if (is_ip)
            entry->un.figs.inippcount++;
    } else {
        entry->un.figs.outpcount++;
        entry->un.figs.outbcount += pktsize;
        entry->un.figs.outspanbr += pktsize;
        if (is_ip)
            entry->un.figs.outippcount++;
    }
}

void printethent(struct ethtab *table, struct ethtabent *entry,
                 unsigned int idx)
{
    unsigned int target_row;

    if ((entry->index < idx) || (entry->index > idx + LINES - 5))
        return;

    target_row = entry->index - idx;

    if (entry->type == 0) {
        wmove(table->tabwin, target_row, 1);
        wattrset(table->tabwin, STDATTR);

        if (entry->un.desc.linktype == LINK_ETHERNET)
            wprintw(table->tabwin, "Ethernet");
        else if (entry->un.desc.linktype == LINK_PLIP)
            wprintw(table->tabwin, "PLIP");
        else if (entry->un.desc.linktype == LINK_FDDI)
            wprintw(table->tabwin, "FDDI");

        wprintw(table->tabwin, " HW addr: %s", entry->un.desc.ascaddr);

        if (entry->un.desc.withdesc)
            wprintw(table->tabwin, " (%s)", entry->un.desc.desc);

        wprintw(table->tabwin, " on %s       ", entry->un.desc.ifname);

        entry->un.desc.printed = 1;
    } else {
        wattrset(table->tabwin, PTRATTR);
        wmove(table->tabwin, target_row, 1);
        waddch(table->tabwin, ACS_LLCORNER);

        wattrset(table->tabwin, HIGHATTR);

        /* Inbound traffic counts */

        wmove(table->tabwin, target_row, 2 * COLS / 80);
        printlargenum(entry->un.figs.inpcount, table->tabwin);
        wmove(table->tabwin, target_row, 12 * COLS / 80);
        printlargenum(entry->un.figs.inippcount, table->tabwin);
        wmove(table->tabwin, target_row, 22 * COLS / 80);
        printlargenum(entry->un.figs.inbcount, table->tabwin);

        /* Outbound traffic counts */

        wmove(table->tabwin, target_row, 40 * COLS / 80);
        printlargenum(entry->un.figs.outpcount, table->tabwin);
        wmove(table->tabwin, target_row, 50 * COLS / 80);
        printlargenum(entry->un.figs.outippcount, table->tabwin);
        wmove(table->tabwin, target_row, 60 * COLS / 80);
        printlargenum(entry->un.figs.outbcount, table->tabwin);
    }
}

void destroyethtab(struct ethtab *table)
{
    struct ethtabent *ptemp = table->head;
    struct ethtabent *cnext = NULL;

    if (table->head != NULL)
        cnext = table->head->next_entry;

    while (ptemp != NULL) {
        free(ptemp);
        ptemp = cnext;

        if (cnext != NULL)
            cnext = cnext->next_entry;
    }
}

void hostmonhelp()
{
    move(LINES - 1, 1);
    scrollkeyhelp();
    sortkeyhelp();
    stdexitkeyhelp();
}

void printrates(struct ethtab *table, unsigned int target_row,
                struct ethtabent *ptmp)
{
    if (ptmp->un.figs.past5) {
        wmove(table->tabwin, target_row, 32 * COLS / 80);
        wprintw(table->tabwin, "%8.1f", ptmp->un.figs.inrate);
        wmove(table->tabwin, target_row, 69 * COLS / 80);
        wprintw(table->tabwin, "%8.1f", ptmp->un.figs.outrate);
    }
}

void updateethrates(struct ethtab *table, int unit, time_t starttime,
                    time_t now, unsigned int idx)
{
    struct ethtabent *ptmp = table->head;
    unsigned int target_row = 0;

    if (table->lastvisible == NULL)
        return;

    while (ptmp != NULL) {
        if (ptmp->type == 1) {
            ptmp->un.figs.past5 = 1;
            if (unit == KBITS) {
                ptmp->un.figs.inrate =
                    ((float) (ptmp->un.figs.inspanbr * 8 / 1000)) /
                    ((float) (now - starttime));
                ptmp->un.figs.outrate =
                    ((float) (ptmp->un.figs.outspanbr * 8 / 1000)) /
                    ((float) (now - starttime));
            } else {
                ptmp->un.figs.inrate =
                    ((float) (ptmp->un.figs.inspanbr / 1024)) /
                    ((float) (now - starttime));
                ptmp->un.figs.outrate =
                    ((float) (ptmp->un.figs.outspanbr / 1024)) /
                    ((float) (now - starttime));
            }
            if ((ptmp->index >= idx) && (ptmp->index <= idx + LINES - 5)) {
                wattrset(table->tabwin, HIGHATTR);
                target_row = ptmp->index - idx;
                printrates(table, target_row, ptmp);
            }
            ptmp->un.figs.inspanbr = ptmp->un.figs.outspanbr = 0;
        }
        ptmp = ptmp->next_entry;
    }
}

void refresh_hostmon_screen(struct ethtab *table, int idx)
{
    struct ethtabent *ptmp = table->firstvisible;

    wattrset(table->tabwin, STDATTR);
    tx_colorwin(table->tabwin);

    while ((ptmp != NULL) && (ptmp->prev_entry != table->lastvisible)) {
        printethent(table, ptmp, idx);
        ptmp = ptmp->next_entry;
    }

    update_panels();
    doupdate();
}

void scrollethwin(struct ethtab *table, int direction, int *idx)
{
    char sp_buf[10];
    sprintf(sp_buf, "%%%dc", COLS - 2);
    wattrset(table->tabwin, STDATTR);
    if (direction == SCROLLUP) {
        if (table->lastvisible != table->tail) {
            wscrl(table->tabwin, 1);
            table->lastvisible = table->lastvisible->next_entry;
            table->firstvisible = table->firstvisible->next_entry;
            (*idx)++;
            wmove(table->tabwin, LINES - 5, 0);
            scrollok(table->tabwin, 0);
            wprintw(table->tabwin, sp_buf, ' ');
            scrollok(table->tabwin, 1);
            printethent(table, table->lastvisible, *idx);
            if (table->lastvisible->type == 1)
                printrates(table, LINES - 5, table->lastvisible);
        }
    } else {
        if (table->firstvisible != table->head) {
            wscrl(table->tabwin, -1);
            table->lastvisible = table->lastvisible->prev_entry;
            table->firstvisible = table->firstvisible->prev_entry;
            (*idx)--;
            wmove(table->tabwin, 0, 0);
            wprintw(table->tabwin, sp_buf, ' ');
            printethent(table, table->firstvisible, *idx);
            if (table->firstvisible->type == 1)
                printrates(table, 0, table->firstvisible);
        }
    }
}

void pageethwin(struct ethtab *table, int direction, int *idx)
{
    int i = 1;

    if (direction == SCROLLUP) {
        while ((i <= LINES - 7) && (table->lastvisible != table->tail)) {
            i++;
            table->firstvisible = table->firstvisible->next_entry;
            table->lastvisible = table->lastvisible->next_entry;
            (*idx)++;
        }
    } else {
        while ((i <= LINES - 7) && (table->firstvisible != table->head)) {
            i++;
            table->firstvisible = table->firstvisible->prev_entry;
            table->lastvisible = table->lastvisible->prev_entry;
            (*idx)--;
        }
    }
    refresh_hostmon_screen(table, *idx);
}

void show_hostsort_keywin(WINDOW ** win, PANEL ** panel)
{
    *win = newwin(13, 35, (LINES - 10) / 2, COLS - 40);
    *panel = new_panel(*win);

    wattrset(*win, DLGBOXATTR);
    tx_colorwin(*win);
    tx_box(*win, ACS_VLINE, ACS_HLINE);

    wattrset(*win, DLGTEXTATTR);
    mvwprintw(*win, 2, 2, "Select sort criterion");
    wmove(*win, 4, 2);
    tx_printkeyhelp("P", " - total packets in", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 5, 2);
    tx_printkeyhelp("I", " - IP packets in", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 6, 2);
    tx_printkeyhelp("B", " - total bytes in", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 7, 2);
    tx_printkeyhelp("K", " - total packets out", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 8, 2);
    tx_printkeyhelp("O", " - IP packets out", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 9, 2);
    tx_printkeyhelp("Y", " - total bytes out", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 10, 2);
    tx_printkeyhelp("Any other key", " - cancel sort", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    update_panels();
    doupdate();
}

/*
 * Swap two host table entries.
 */

void swaphostents(struct ethtab *list, struct ethtabent *p1,
                  struct ethtabent *p2)
{
    register unsigned int tmp;
    struct ethtabent *p1prevsaved;
    struct ethtabent *p2nextsaved;

    if (p1 == p2)
        return;

    tmp = p1->index;
    p1->index = p2->index;
    p2->index = tmp;
    p1->next_entry->index = p1->index + 1;
    p2->next_entry->index = p2->index + 1;

    if (p1->prev_entry != NULL)
        p1->prev_entry->next_entry = p2;
    else
        list->head = p2;

    if (p2->next_entry->next_entry != NULL)
        p2->next_entry->next_entry->prev_entry = p1->next_entry;
    else
        list->tail = p1->next_entry;

    p2nextsaved = p2->next_entry->next_entry;
    p1prevsaved = p1->prev_entry;

    if (p1->next_entry->next_entry == p2) {
        p2->next_entry->next_entry = p1;
        p1->prev_entry = p2->next_entry;
    } else {
        p2->next_entry->next_entry = p1->next_entry->next_entry;
        p1->prev_entry = p2->prev_entry;
        p2->prev_entry->next_entry = p1;
        p1->next_entry->next_entry->prev_entry = p2->next_entry;
    }

    p2->prev_entry = p1prevsaved;
    p1->next_entry->next_entry = p2nextsaved;
}

unsigned long long ql_getkey(struct ethtabent *entry, int ch)
{
    unsigned long long result = 0;

    switch (ch) {
    case 'P':
        result = entry->next_entry->un.figs.inpcount;
        break;
    case 'I':
        result = entry->next_entry->un.figs.inippcount;
        break;
    case 'B':
        result = entry->next_entry->un.figs.inbcount;
        break;
    case 'K':
        result = entry->next_entry->un.figs.outpcount;
        break;
    case 'O':
        result = entry->next_entry->un.figs.outippcount;
        break;
    case 'Y':
        result = entry->next_entry->un.figs.outbcount;
        break;
    }
    return result;
}

struct ethtabent *ql_partition(struct ethtab *table,
                               struct ethtabent **low,
                               struct ethtabent **high, int ch)
{
    struct ethtabent *pivot = *low;

    struct ethtabent *left = *low;
    struct ethtabent *right = *high;
    struct ethtabent *ptmp;

    unsigned long long pivot_value;

    pivot_value = ql_getkey(pivot, ch);

    while (left->index < right->index) {
        while ((ql_getkey(left, ch) >= pivot_value)
               && (left->next_entry->next_entry != NULL))
            left = left->next_entry->next_entry;

        while (ql_getkey(right, ch) < pivot_value)
            right = right->prev_entry->prev_entry;

        if (left->index < right->index) {
            swaphostents(table, left, right);

            if (*low == left)
                *low = right;

            if (*high == right)
                *high = left;

            ptmp = left;
            left = right;
            right = ptmp;
        }
    }
    swaphostents(table, pivot, right);

    if (*low == pivot)
        *low = right;

    if (*high == right)
        *high = pivot;

    return pivot;
}

/*
 * Quicksort routine for the LAN station monitor
 */

void quicksort_lan_entries(struct ethtab *table, struct ethtabent *low,
                           struct ethtabent *high, int ch)
{
    struct ethtabent *pivot;

    if ((high == NULL) || (low == NULL))
        return;

    if (high->index > low->index) {
        pivot = ql_partition(table, &low, &high, ch);

        if (pivot->prev_entry != NULL)
            quicksort_lan_entries(table, low,
                                  pivot->prev_entry->prev_entry, ch);

        quicksort_lan_entries(table, pivot->next_entry->next_entry, high,
                              ch);
    }
}

void sort_hosttab(struct ethtab *list, int *idx, int command)
{
    struct ethtabent *ptemp1;
    unsigned int idxtmp;

    if (!list->head)
        return;

    command = toupper(command);

    if ((command != 'P') && (command != 'I') && (command != 'B') &&
        (command != 'K') && (command != 'O') && (command != 'Y'))
        return;

    quicksort_lan_entries(list, list->head, list->tail->prev_entry,
                          command);

    ptemp1 = list->firstvisible = list->head;
    *idx = 1;
    idxtmp = 0;
    tx_colorwin(list->tabwin);
    while ((ptemp1) && (idxtmp <= LINES - 4)) {
        printethent(list, ptemp1, *idx);
        idxtmp++;
        if (idxtmp <= LINES - 4)
            list->lastvisible = ptemp1;
        ptemp1 = ptemp1->next_entry;
    }

}

/*
 * The LAN station monitor
 */

void hostmon(const struct OPTIONS *options, int facilitytime, char *ifptr,
             struct filterstate *ofilter)
{
    int logging = options->logging;
    int fd;
    struct ethtab table;
    struct ethtabent *entry;
    struct sockaddr_ll fromaddr;
    unsigned short linktype;

    int br;
    char buf[MAX_PACKET_SIZE];
    char scratch_saddr[6];
    char scratch_daddr[6];
    unsigned int idx = 1;
    int is_ip;
    int ch;

    char ifname[10];

    struct timeval tv;
    unsigned long starttime;
    unsigned long now = 0;
    unsigned long long unow = 0;
    unsigned long statbegin = 0, startlog = 0;
    unsigned long updtime = 0;
    unsigned long long updtime_usec = 0;

    struct desclist elist;      /* Ethernet description list */
    struct desclist flist;      /* FDDI description list */
    struct desclist *list = NULL;

    FILE *logfile = NULL;

    int pkt_result;
    char *ipacket;

    int nomem = 0;

    WINDOW *sortwin;
    PANEL *sortpanel;
    int keymode = 0;

    int instance_id;
    char msgstring[80];

    struct promisc_states *promisc_list;

    if (!facility_active(LANMONIDFILE, ifptr))
        mark_facility(LANMONIDFILE, "LAN monitor", ifptr);
    else {
        snprintf(msgstring, 80,
                 "LAN station monitor already running on %s",
                 gen_iface_msg(ifptr));
        write_error(msgstring, daemonized);
        return;
    }

    if (ifptr != NULL) {
        if (!iface_supported(ifptr)) {
            err_iface_unsupported();
            unmark_facility(LANMONIDFILE, ifptr);
            return;
        }
        if (!iface_up(ifptr)) {
            err_iface_down();
            unmark_facility(LANMONIDFILE, ifptr);
            return;
        }
    }

    open_socket(&fd);

    if (fd < 0) {
        unmark_facility(LANMONIDFILE, ifptr);
        return;
    }

    if ((first_active_facility()) && (options->promisc)) {
        init_promisc_list(&promisc_list);
        save_promisc_list(promisc_list);
        srpromisc(1, promisc_list);
        destroy_promisc_list(&promisc_list);
    }

    adjust_instance_count(PROCCOUNTFILE, 1);
    instance_id = adjust_instance_count(LANMONCOUNTFILE, 1);
    strncpy(active_facility_countfile, LANMONCOUNTFILE, 64);

    hostmonhelp();

    initethtab(&table, options->actmode);
    loaddesclist(&elist, LINK_ETHERNET, WITHETCETHERS);
    loaddesclist(&flist, LINK_FDDI, WITHETCETHERS);

    if (logging) {
        if (strcmp(current_logfile, "") == 0) {
            strncpy(current_logfile,
                    gen_instance_logname(LANLOG, instance_id), 80);

            if (!daemonized)
                input_logfile(current_logfile, &logging);
        }
    }

    if (logging) {
        opentlog(&logfile, current_logfile);

        if (logfile == NULL)
            logging = 0;
    }
    if (logging)
        signal(SIGUSR1, rotate_lanlog);

    rotate_flag = 0;
    writelog(logging, logfile,
             "******** LAN traffic monitor started ********");
    leaveok(table.tabwin, TRUE);

    exitloop = 0;
    gettimeofday(&tv, NULL);
    starttime = statbegin = startlog = tv.tv_sec;

    do {
        gettimeofday(&tv, NULL);
        now = tv.tv_sec;
        unow = tv.tv_sec * 1e+6 + tv.tv_usec;

        if ((now - starttime) >= 5) {
            printelapsedtime(statbegin, now, LINES - 3, 15,
                             table.borderwin);
            updateethrates(&table, options->actmode, starttime, now, idx);
            starttime = now;
        }
        if (((now - startlog) >= options->logspan) && (logging)) {
            writeethlog(table.head, options->actmode, now - statbegin,
                        logfile);
            startlog = now;
        }
        if (((options->updrate != 0)
             && (now - updtime >= options->updrate))
            || ((options->updrate == 0)
                && (unow - updtime_usec >= HOSTMON_UPDATE_DELAY))) {
            update_panels();
            doupdate();
            updtime = now;
            updtime_usec = unow;
        }
        check_rotate_flag(&logfile, logging);

        if ((facilitytime != 0)
            && (((now - statbegin) / 60) >= facilitytime))
            exitloop = 1;

        getpacket(fd, buf, &fromaddr, &ch, &br, ifname, table.tabwin);

        if (ch != ERR) {
            if (keymode == 0) {
                switch (ch) {
                case KEY_UP:
                    scrollethwin(&table, SCROLLDOWN, &idx);
                    break;
                case KEY_DOWN:
                    scrollethwin(&table, SCROLLUP, &idx);
                    break;
                case KEY_PPAGE:
                case '-':
                    pageethwin(&table, SCROLLDOWN, &idx);
                    break;
                case KEY_NPAGE:
                case ' ':
                    pageethwin(&table, SCROLLUP, &idx);
                    break;
                case 12:
                case 'l':
                case 'L':
                    tx_refresh_screen();
                    break;
                case 's':
                case 'S':
                    show_hostsort_keywin(&sortwin, &sortpanel);
                    keymode = 1;
                    break;
                case 'q':
                case 'Q':
                case 'x':
                case 'X':
                case 27:
                case 24:
                    exitloop = 1;
                }
            } else if (keymode == 1) {
                del_panel(sortpanel);
                delwin(sortwin);
                sort_hosttab(&table, &idx, ch);
                keymode = 0;
            }
        }
        if (br > 0) {
            pkt_result = processpacket(buf, &ipacket, &br, NULL,
                                       NULL, NULL, &fromaddr, &linktype,
                                       ofilter, MATCH_OPPOSITE_USECONFIG,
                                       ifname, ifptr);

            if (pkt_result != PACKET_OK)
                continue;

            if ((linktype == LINK_ETHERNET) || (linktype == LINK_FDDI)
                || (linktype == LINK_PLIP) || (linktype == LINK_TR) ||
                (linktype == LINK_VLAN)) {

                if (fromaddr.sll_protocol == htons(ETH_P_IP))
                    is_ip = 1;
                else
                    is_ip = 0;

                /*
                 * Check source address entry
                 */

                if ((linktype == LINK_ETHERNET) || (linktype == LINK_PLIP)
                    || (linktype == LINK_VLAN)) {
                    memcpy(scratch_saddr,
                           ((struct ethhdr *) buf)->h_source, ETH_ALEN);
                    memcpy(scratch_daddr, ((struct ethhdr *) buf)->h_dest,
                           ETH_ALEN);
                    list = &elist;
                } else if (linktype == LINK_FDDI) {
                    memcpy(scratch_saddr, ((struct fddihdr *) buf)->saddr,
                           FDDI_K_ALEN);
                    memcpy(scratch_daddr, ((struct fddihdr *) buf)->daddr,
                           FDDI_K_ALEN);
                    list = &flist;
                } else if (linktype == LINK_TR) {
                    memcpy(scratch_saddr, ((struct trh_hdr *) buf)->saddr,
                           TR_ALEN);
                    memcpy(scratch_daddr, ((struct trh_hdr *) buf)->daddr,
                           TR_ALEN);
                    list = &flist;
                }

                entry = in_ethtable(&table, linktype, scratch_saddr);

                if ((entry == NULL) && (!nomem))
                    entry =
                        addethentry(&table, linktype, ifname,
                                    scratch_saddr, &nomem, list);

                if (entry != NULL) {
                    updateethent(entry, br, is_ip, 1);
                    if (!entry->prev_entry->un.desc.printed)
                        printethent(&table, entry->prev_entry, idx);

                    printethent(&table, entry, idx);
                }
                /*
                 * Check destination address entry
                 */

                entry = in_ethtable(&table, linktype, scratch_daddr);
                if ((entry == NULL) && (!nomem))
                    entry =
                        addethentry(&table, linktype, ifname,
                                    scratch_daddr, &nomem, list);

                if (entry != NULL) {
                    updateethent(entry, br, is_ip, 0);
                    if (!entry->prev_entry->un.desc.printed)
                        printethent(&table, entry->prev_entry, idx);

                    printethent(&table, entry, idx);
                }
            }
        }
    } while (!exitloop);

    close(fd);

    if ((options->promisc) && (is_last_instance())) {
        load_promisc_list(&promisc_list);
        srpromisc(0, promisc_list);
        destroy_promisc_list(&promisc_list);
    }

    adjust_instance_count(PROCCOUNTFILE, -1);
    adjust_instance_count(LANMONCOUNTFILE, -1);

    if (logging) {
        signal(SIGUSR1, SIG_DFL);
        writeethlog(table.head, options->actmode,
                    time((time_t *) NULL) - statbegin, logfile);
        writelog(logging, logfile,
                 "******** LAN traffic monitor stopped ********");
        fclose(logfile);
    }


    del_panel(table.tabpanel);
    delwin(table.tabwin);
    del_panel(table.borderpanel);
    delwin(table.borderwin);
    update_panels();
    doupdate();
    destroyethtab(&table);
    destroydesclist(&elist);
    destroydesclist(&flist);
    unmark_facility(LANMONIDFILE, ifptr);
    strcpy(current_logfile, "");
}
